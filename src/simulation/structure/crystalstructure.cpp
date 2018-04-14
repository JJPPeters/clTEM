#include "crystalstructure.h"

#include <fstream>
#include <time.h>

#include "utilities/stringutils.h"
#include "utilities/structureutils.h"

CrystalStructure::CrystalStructure(std::string fPath) : ScaleFactor(1.0), AtomCount(0)//, Sorted(false)
{
    resetLimits();
    Atoms = std::vector<Atom>();

    srand(time(NULL));
    openXyz(fPath);

    std::random_device rd;
    rng = std::mt19937(rd());
    dist = std::normal_distribution<>(0, 1);
}

void CrystalStructure::openXyz(std::string fPath)
{
    filePath = fPath;
    // open the file
    std::ifstream inputStream;
    inputStream.open(filePath);
    if(!inputStream)
        throw std::runtime_error("Error opening .xyz file.");

    filePath = fPath;

    bool useOccupancy = false;

    // this is our buffer, contains the current line only
    std::string line;

    // get the first line and set it as the number of atoms
    Utils::safeGetline(inputStream, line);
    AtomCount = std::stoi(line);

    // get the next line, search it for our comment tags
    Utils::safeGetline(inputStream, line);
    std::size_t found = line.find("nm");
    if(found!=std::string::npos) //TODO: this should just be one consistent value
        ScaleFactor = 10;
    found = line.find("occ");
    if(found!=std::string::npos)
        useOccupancy = true;

    AtomOcc thisAtom;
    std::vector<AtomOcc> prevAtoms;
    prevAtoms.reserve(5); //this array will be resized a lot so reserve space. 5 should be plenty for any atoms sharing same sites

    int count = 0;

    // here we actually loop through the lines and get the atoms...
    if(useOccupancy)
    {
        // loop throguh all atoms
        std::string atomSymbol;
        while(inputStream >> atomSymbol >> thisAtom.x >> thisAtom.y >> thisAtom.z >> thisAtom.occ)
        {
            if(atomSymbol.size() < 1) // this handles newlines at end of file etc...
                break;
            thisAtom.A = Utils::ElementSymbolToNumber(atomSymbol);

            // this can get very confusing, but what we do is build a list of atoms that are at the same site
            // then we process them when we have a full list of atoms on that site
            // first test if we have a list of atoms sharing a site
            if(prevAtoms.size() > 0) // we do!
            {
                // we have a list of atoms, if this new one is in the same place then we just add it
                if (prevAtoms[0] == (Atom)thisAtom)
                {
                    prevAtoms.push_back(thisAtom);
                }
                else // this means we have a complete array and we need to process it
                {
                    processOccupancyList(prevAtoms); // this clears the vector
                    prevAtoms.push_back(thisAtom);
                }
            }
            else // we don't, so we make it here!
            {
                prevAtoms.push_back(thisAtom);
            }
            ++count;
        }

        // the last set doesnt actually get processed so we do it now
        processOccupancyList(prevAtoms);
    }
    else //technically don't need this but it will speed up the cases with no occupancy a lot
    {
        std::string atomSymbol;
        while(inputStream >> atomSymbol >> thisAtom.x >> thisAtom.y >> thisAtom.z)
        {
            if(atomSymbol.size() < 1) // this handles newlines at end of file etc...
                break;
            thisAtom.A = Utils::ElementSymbolToNumber(atomSymbol);
            Atoms.push_back(thisAtom * ScaleFactor);
            updateLimits(thisAtom * ScaleFactor);
            ++count;
        }
    }

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

void CrystalStructure::processOccupancyList(std::vector<AtomOcc> &aList)
{
    if (aList.size() == 1 and aList[0].occ == 1.0) // small try at optimising
    {
        Atoms.push_back(aList[0] * ScaleFactor);
        updateLimits(aList[0] * ScaleFactor);
    }
    else
    {
        double r = ((double) rand() / (RAND_MAX));
        float totalOcc = 0.0;

        for(auto a : aList)
        {
            if((r >= totalOcc && r < totalOcc+a.occ) || (r == 1.0 && totalOcc+a.occ == 1.0))
            {
                Atoms.push_back(a * ScaleFactor);
                updateLimits(a * ScaleFactor);
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

float CrystalStructure::generateTdsFactor()
{
    // TODO: check this behaves as expected, may want to reset the random stuff
    float randNormal = 0.075f * (float) dist(rng);

    return randNormal;
}

//float CrystalStructure::getSimZRange()
//{
//    return getSimMaxZ() - getSimMinZ();
//}

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

//std::tuple<float, float> CrystalStructure::getSimRanges()
//{
//    return std::make_tuple(getSimMaxX()- getSimMinX(), getSimMaxY()- getSimMinY());
//}

std::tuple<float, float> CrystalStructure::getStructRanges()
{
    return std::make_tuple( MaxX-MinX, MaxY-MinY);
}



















