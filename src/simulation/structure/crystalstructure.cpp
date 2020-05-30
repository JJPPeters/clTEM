#include <utility>

#include "crystalstructure.h"

#include <fstream>
#include <ctime>
#include <utilities/vectorutils.h>
#include <chrono>

#include "utilities/stringutils.h"
#include "utilities/structureutils.h"

CrystalStructure::CrystalStructure(std::string &fPath, CIF::SuperCellInfo info, bool fix_cif)
        : scale_factor(1.0), atom_count(0), file_defined_thermals(false), max_atomic_number(0) {
    // create our random number stuffs
    dist = std::normal_distribution<>(0, 1);
    rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());

    resetLimits();
    atom_list = std::vector<AtomSite>();

    std::string ext = fPath.substr(fPath.length() - 4);

    if (ext == ".xyz")
        openXyz(fPath);
    else if (ext == ".cif") {
        openCif(fPath, info, fix_cif);
    }
}

CrystalStructure::CrystalStructure(CIF::CIFReader cif, CIF::SuperCellInfo info)
        : scale_factor(1.0), atom_count(0), file_defined_thermals(false), max_atomic_number(0) {
    // create our random number stuffs
    dist = std::normal_distribution<>(0, 1);
    rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());

    resetLimits();
    atom_list = std::vector<AtomSite>();

    openCif(cif, info);
}

void CrystalStructure::openXyz(std::string fPath) {
    file_path = std::move(fPath);
    // open the file
    std::ifstream inputStream;
    inputStream.open(file_path);
    if(!inputStream)
        throw std::runtime_error("Error opening .xyz file.");

    // this is our buffer, contains the current line only
    std::string line;

    // get the first line and set it as the number of atoms
    Utils::safeGetline(inputStream, line);
    size_t atom_count;
    try {
        atom_count = std::stoul(line);
    } catch (const std::exception &e) {
        throw std::runtime_error("Could not parse number of atoms (line 1, " + std::string(e.what()) + ").");
    }
//    Atoms.reserve(AtomCount);

    // get the next line, in my format, this contains the column info
    Utils::safeGetline(inputStream, line);

    // split this line by whitespace
    auto headers = Utils::splitStringSpace(line);

    // find and remove the 'nm' modifier tag whilst settings the scale factor
    if (Utils::findItemIndex(headers, std::string("nm")) != -1){
        scale_factor = 10;
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
//    int header_count = 4 + (h_occ >= 0) + (h_u >= 0) + (h_ux >= 0) + (h_uy >= 0) + (h_uz >= 0);

    // TODO: report warning on unused headers?

    std::vector<std::string> A(atom_count);
    std::vector<double> x(atom_count);
    std::vector<double> y(atom_count);
    std::vector<double> z(atom_count);

    // These are only resized when we have the headers as a way of calculating if they are used or not
    std::vector<double> occ;
    if (h_occ != -1)
        occ.resize(atom_count);

    std::vector<double> ux;
    std::vector<double> uy;
    std::vector<double> uz;
    std::vector<bool> def_u(atom_count);
    bool defined_thermals = h_u != -1 || h_ux != -1 || h_uy != -1 || h_uz != -1;
    if (defined_thermals) {
        ux.resize(atom_count);
        uy.resize(atom_count);
        uz.resize(atom_count);
    }

    std::string temp_line;
    int i = 0;
    while(!Utils::safeGetline(inputStream, temp_line).eof()) {
        if (temp_line.empty()) // this handles newlines at end of file etc... I think...
            break;

        if (i > atom_count)
            throw std::runtime_error("Number of atoms does not match .xyz first line (" + Utils::numToString(atom_count) + ")");

        auto values = Utils::splitStringSpace(temp_line);

        if (values.size() < max_header)
            throw std::runtime_error(".xyz file columns are fewer than header entries. line: " + std::to_string(2 + i));

        A[i] = values[h_A];
        x[i] = std::stof(values[h_x]);
        y[i] = std::stof(values[h_y]);
        z[i] = std::stof(values[h_z]);

        // I think I could just leave this empty, and it will work, bit I'm setting it incase I need it later
        def_u[i] = defined_thermals;

        if (h_occ != -1) // if this isn't present, it is defaulted to 1 in the constructor
            occ[i] = std::stof(values[h_occ]);

        if (h_u != -1) {
            ux[i] = std::stof(values[h_u]);
            uy[i] = std::stof(values[h_u]);
            uz[i] = std::stof(values[h_u]);
        }
        // these override the isotropic value
        if (h_ux != -1)
            ux[i] = std::stof(values[h_ux]);
        if (h_uy != -1)
            uy[i] = std::stof(values[h_uy]);
        if (h_uz != -1)
            uz[i] = std::stof(values[h_z]);

        ++i;
    }

    inputStream.close();
    if (i != atom_count)
        throw std::runtime_error("Number of atoms does not match .xyz first line: " + Utils::numToString(i) + " instead of " + Utils::numToString(atom_count));

    // now have a list of ALL our values, process them (i.e. occupancies) in this next function
    processAtomList(A, x, y, z, occ, def_u, ux, uy, uz);
}

void CrystalStructure::openCif(std::string fPath, CIF::SuperCellInfo info, bool fix_cif) {
    openCif(CIF::CIFReader(fPath, fix_cif), info);
}

void CrystalStructure::openCif(CIF::CIFReader cif, CIF::SuperCellInfo info) {
    // open our cif here
    file_path = cif.getFilePath();

    // need to create the vectors the data will be put into
    std::vector<std::string> A;
    std::vector<double> x, y, z, occ, ux, uy, uz;
    std::vector<bool> def_u;


    CIF::makeSuperCell(cif, info, A, x, y, z, occ, def_u, ux, uy, uz);

    processAtomList(A, x, y, z, occ, def_u, ux, uy, uz);
}

void CrystalStructure::processOccupancyList(std::vector<AtomSite> &aList)
{
    if (aList.empty())
        return;

    if (aList.size() == 1 and aList[0].occ == 1.0) {// small try at optimising
        addAtom(aList[0]);
    } else {
        double r = dist(rng);
        double totalOcc = 0.0;

        for(auto a : aList) {
            if((r >= totalOcc && r < totalOcc+a.occ) || (r == 1.0 && totalOcc+a.occ == 1.0))
                addAtom(a);
            totalOcc += a.occ;
        }

        if(totalOcc > 1.0000001) // decimal places to account for floating point errors
            throw std::runtime_error(".xyz has occupancies > 1.");
    }

    aList.clear();
}

void CrystalStructure::updateLimits(const Atom &a)
{
    if (a.x > max_x)
        max_x = a.x;
    if (a.y > max_y)
        max_y = a.y;
    if (a.z > max_z)
        max_z = a.z;
    if (a.x < min_x)
        min_x = a.x;
    if (a.y < min_y)
        min_y = a.y;
    if (a.z < min_z)
        min_z = a.z;

    if (a.A > max_atomic_number)
        max_atomic_number = a.A;
}

void CrystalStructure::resetLimits()
{
    min_x = std::numeric_limits<double>::max();
    max_x = std::numeric_limits<double>::min();

    min_y = std::numeric_limits<double>::max();
    max_y = std::numeric_limits<double>::min();

    min_z = std::numeric_limits<double>::max();
    max_z = std::numeric_limits<double>::min();
}

int CrystalStructure::atomCountInRange(double xs, double xf, double ys, double yf)
{
    // this might be stupidly slow, but it's nice to know how many atoms you are actually simulating through
    // TODO: could subtract the min values when first opening the structure somehow???

    int count = 0;
    for (auto a : atom_list)
        if (a.x - min_x >= xs && a.x - min_x <= xf && a.y - min_y >= ys && a.y - min_y <= yf)
            ++count;

    return count;
}

void CrystalStructure::addAtom(AtomSite a) {
    // add the atom
    atom_list.emplace_back(a * scale_factor);
    // update limits
    updateLimits(a * scale_factor);
    // update our list of atoms
//    if(std::find(AtomTypes.begin(), AtomTypes.end(), a.A) == AtomTypes.end())
//        AtomTypes.push_back(a.A);
}

void CrystalStructure::processAtomList(std::vector<std::string> A, std::vector<double> x, std::vector<double> y, std::vector<double> z, std::vector<double> occ, std::vector<bool> def_u, std::vector<double> ux, std::vector<double> uy, std::vector<double> uz) {

    // TODO: error is sizes not all the same
    size_t count = A.size();
    if (x.size() != count || y.size() != count || z.size() != count)
        throw std::runtime_error("Processing atom list with unequal length vectors");

    bool use_occ = !occ.empty();
    if (use_occ && occ.size() != count)
        throw std::runtime_error("Processing atom list with unequal length vectors");

    file_defined_thermals = std::find(begin(def_u), end(def_u), false) == end(def_u);

    if (file_defined_thermals && (ux.size() != count || uy.size() != count || uz.size() != count))
        throw std::runtime_error("Processing atom list with unequal length vectors");

    atom_list.reserve(count);

    std::vector<AtomSite> prevAtoms;
    prevAtoms.reserve(10); //this array will be resized a lot so reserve space. 10 should be plenty for any atoms sharing same sites

    for (int i = 0; i < count; ++i) {
        AtomSite thisAtom;

        thisAtom.defined_u = def_u[i];

        thisAtom.A = Utils::ElementSymbolToNumber(A[i]);
        thisAtom.x = x[i];
        thisAtom.y = y[i];
        thisAtom.z = z[i];

        if (use_occ)
            thisAtom.occ = occ[i];

        // for loading cif files, the ux etc will have valid 0 values, but for xyz files they could be empty vectors
        if (def_u[i]) {
            thisAtom.ux = ux[i];
            thisAtom.uy = uy[i];
            thisAtom.uz = uz[i];
        }

        if (use_occ) {
            // this can get very confusing, but what we do is build a list of atoms that are at the same site
            // then we process them when we have a full list of atoms on that sit

            // first test if we have no previous atoms, or this atom belongs in that list
            if (prevAtoms.empty() || prevAtoms[0] == thisAtom) {
                prevAtoms.push_back(thisAtom); // simple add to our list
            } else {
                processOccupancyList(prevAtoms); // this processes the previos atoms and clears the vector
                prevAtoms.push_back(thisAtom); // start our new list
            }
        } else {
            addAtom(thisAtom); // for occ, this all happens in the processOccupancyList method
        }
    }

    if (!prevAtoms.empty())
        processOccupancyList(prevAtoms);
}



















