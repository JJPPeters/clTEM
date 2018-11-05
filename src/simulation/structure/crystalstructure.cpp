#include "crystalstructure.h"

#include <fstream>
#include <ctime>
#include <utilities/vectorutils.h>

#include "utilities/stringutils.h"
#include "utilities/structureutils.h"

CrystalStructure::CrystalStructure(std::string& fPath) : ScaleFactor(1.0), AtomCount(0), file_defined_thermals(false)
{
    resetLimits();
    Atoms = std::vector<AtomSite>();

    openXyz(fPath);
}

void CrystalStructure::openXyz(std::string fPath)
{
    filePath = fPath;
    // open the file
    std::ifstream inputStream;
    inputStream.open(filePath);
    if(!inputStream)
        throw std::runtime_error("Error opening .xyz file.");

    // this is our buffer, contains the current line only
    std::string line;

    // get the first line and set it as the number of atoms
    Utils::safeGetline(inputStream, line);
    try {
        AtomCount = std::stoi(line);
    } catch (const std::exception &e) {
        throw std::runtime_error("Could not parse number of atoms (line 1, " + std::string(e.what()) + ").");
    }
    Atoms.reserve(AtomCount);

    // get the next line, in my format, this contains the column info
    Utils::safeGetline(inputStream, line);

    // split this line by whitespace
    auto headers = Utils::splitStringSpace(line);

    // find and remove the 'nm' modifier tag whilst settings the scale factor
    if (Utils::findItemIndex(headers, std::string("nm")) != -1){
        ScaleFactor = 10;
        headers.erase(std::remove(headers.begin(), headers.end(), "nm"), headers.end());
    }

    // find the index of possible header values

    // find the indices
    int h_A = Utils::findItemIndex(headers, std::string("A"));
    int h_x = Utils::findItemIndex(headers, std::string("x"));
    int h_y = Utils::findItemIndex(headers, std::string("y"));
    int h_z = Utils::findItemIndex(headers, std::string("z"));

    int h_occ = Utils::findItemIndex(headers, std::string("occ"));

    int h_u = Utils::findItemIndex(headers, std::string("u"));
    int h_ux = Utils::findItemIndex(headers, std::string("ux"));
    int h_uy = Utils::findItemIndex(headers, std::string("uy"));
    int h_uz = Utils::findItemIndex(headers, std::string("uz"));

    bool default_headers = false;
    // set defaults to A, x, y, z if they ALL don't exist
    if (h_A == -1 && h_x == -1 && h_y == -1 && h_z == -1) {
        h_A = 0;
        h_x = 1;
        h_y = 2;
        h_z = 3;
        default_headers = true;
    }

    // this detects if the required headers are only partially set
    if (h_A == -1 || h_x == -1 || h_y == -1 || h_z == -1)
        throw std::runtime_error(".xyz file headers not complete (requires A, x, y, z)");

    // if we are using default headers, the rest are taken from there (if they exist)
    if (default_headers) {
        h_occ += 4 * (h_occ != -1);
        h_u += 4 * (h_u != -1);
        h_ux += 4 * (h_ux != -1);
        h_uy += 4 * (h_uy != -1);
        h_uz += 4 * (h_uz != -1);
    }

    auto max_header = std::max<int>({h_A, h_x, h_y, h_z, h_occ, h_u, h_ux, h_uy, h_uz});

    file_defined_thermals = h_u != -1 || h_ux != -1 || h_uy != -1 || h_uz != -1;

    // TODO: report warning on unused headers?

    std::vector<AtomSite> prevAtoms;
    prevAtoms.reserve(5); //this array will be resized a lot so reserve space. 5 should be plenty for any atoms sharing same sites

    int count = 0;

    // here we actually loop through the lines and get the atoms...
//    if(h_occ != -1) // if we have occupancy values...
//    {
    // loop through all atoms (lines in the file)
    std::string temp_line;
    while(!Utils::safeGetline(inputStream, temp_line).eof()) {
        if (temp_line.empty()) // this handles newlines at end of file etc... I think...
            break;
        auto values = Utils::splitStringSpace(temp_line);

        if (values.size() < max_header)
            throw std::runtime_error(
                    ".xyz file columns are fewer than header entries. line: " + std::to_string(2 + count));

        AtomSite thisAtom;

        std::string atomSymbol = values[h_A];
        thisAtom.A = Utils::ElementSymbolToNumber(atomSymbol);
        thisAtom.x = std::stof(values[h_x]);
        thisAtom.y = std::stof(values[h_y]);
        thisAtom.z = std::stof(values[h_z]);

        if (h_occ != -1) // if this isn't present, it is defaulted to 1 in the constructor
            thisAtom.occ = std::stof(values[h_occ]);

        if (h_u != -1)
            thisAtom.setThermal(std::stof(values[h_u]));
        // these override the isotropic value
        if (h_ux != -1)
            thisAtom.ux = std::stof(values[h_ux]);
        if (h_uy != -1)
            thisAtom.uy = std::stof(values[h_uy]);
        if (h_uz != -1)
            thisAtom.uz = std::stof(values[h_uz]);

        if (h_occ != -1) {
            // this can get very confusing, but what we do is build a list of atoms that are at the same site
            // then we process them when we have a full list of atoms on that site
            // first test if we have a list of atoms sharing a site
            if (!prevAtoms.empty()) // we do!
            {
                // we have a list of atoms, if this new one is in the same place then we just add it
                if (prevAtoms[0] == thisAtom) {
                    prevAtoms.emplace_back(thisAtom);
                } else // this means we have a complete array and we need to process it
                {
                    processOccupancyList(prevAtoms); // this adds the atoms and clears the vector
                    prevAtoms.emplace_back(thisAtom);
                }
            } else // we don't, so we make it here!
            {
                prevAtoms.emplace_back(thisAtom);
            }
            ++count;
        } else {
            // for TDS this all happens in processOccupancyList
            addAtom(thisAtom);
            ++count;
        }
    }

    // the last set doesnt actually get processed so we do it now
    // only does anything if the occupancy is being used
    processOccupancyList(prevAtoms);

    // can't test Atoms.size() as it won't have all atoms due to occupancy
    if (count != AtomCount)
        throw std::runtime_error("Number of atoms does not match .xyz first line: " + Utils::numToString(Atoms.size()) + " instead of " + Utils::numToString(AtomCount));

    inputStream.close();
}

void CrystalStructure::clearStructure()
{
    resetLimits();
//    Sorted = false;
    Atoms.clear();
}

void CrystalStructure::processOccupancyList(std::vector<AtomSite> &aList)
{
    if (aList.empty())
        return;

    if (aList.size() == 1 and aList[0].occ == 1.0) // small try at optimising
    {
        addAtom(aList[0]);
    }
    else
    {
        double r = ((double) rand() / (RAND_MAX));
        float totalOcc = 0.0;

        for(auto a : aList)
        {
            if((r >= totalOcc && r < totalOcc+a.occ) || (r == 1.0 && totalOcc+a.occ == 1.0))
            {
                addAtom(a);
            }

            totalOcc += a.occ;
        }

        if(totalOcc > 1.0)
            throw std::runtime_error(".xyz has occupancies > 1.");
    }

    aList.clear();
}

void CrystalStructure::updateLimits(const Atom &a)
{
    if (a.x > MaxX)
        MaxX = a.x;
    if (a.y > MaxY)
        MaxY = a.y;
    if (a.z > MaxZ)
        MaxZ = a.z;
    if (a.x < MinX)
        MinX = a.x;
    if (a.y < MinY)
        MinY = a.y;
    if (a.z < MinZ)
        MinZ = a.z;
}

void CrystalStructure::resetLimits()
{
    MinX = std::numeric_limits<float>::max();
    MaxX = std::numeric_limits<float>::min();

    MinY = std::numeric_limits<float>::max();
    MaxY = std::numeric_limits<float>::min();

    MinZ = std::numeric_limits<float>::max();
    MaxZ = std::numeric_limits<float>::min();
}

float CrystalStructure::getZRange()
{
    return MaxZ - MinZ;
}

int CrystalStructure::getAtomCountInRange(float xs, float xf, float ys, float yf)
{
    // this might be stupidly slow, but it's nice to know how many atoms you are actually simulating through
    // TODO: could subtact the min values when first opening the structure somehow???

    int count = 0;
    for (auto a : Atoms)
        if (a.x-MinX > xs && a.x-MinX < xf && a.y-MinY > ys && a.y-MinY < yf)
            ++count;

    return count;
}

std::tuple<float, float> CrystalStructure::getStructRanges()
{
    return std::make_tuple( MaxX-MinX, MaxY-MinY);
}

void CrystalStructure::addAtom(AtomSite a) {
    // add the atom
    Atoms.emplace_back(a * ScaleFactor);
    // update limits
    updateLimits(a * ScaleFactor);
    // update our list of atoms
    if(std::find(AtomTypes.begin(), AtomTypes.end(), a.A) == AtomTypes.end())
        AtomTypes.push_back(a.A);
}



















