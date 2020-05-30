#ifndef CRYSTALSTRUCTURE_H
#define CRYSTALSTRUCTURE_H

#include <vector>
#include <string>
#include <valarray>
#include <random>

#include "utilities/commonstructs.h"

#include "incoherence/inelastic/phonon.h"

#include "atom.h"
#include "cif/cifreader.h"
#include "cif/supercell.h"

namespace FileFormat {
    enum FileFormat {
        XYZ,
        CIF
    };
}

class CrystalStructure
{
private:
    /// Vector of atoms with coordinates in Angstroms
    std::vector<AtomSite> atom_list;

    /// Filepath to to file we opened
    std::string file_path;

    bool file_defined_thermals;

    /// Use to conver the structure/file units to Angstrom
    double scale_factor;

    /// Spacial ;imits of our structure
    double max_x;
    double min_x;
    double max_y;
    double min_y;
    double max_z;
    double min_z;

    /// MAx atomic number - used to see our parameterisation covers this (assumes parameterisation does not have gaps)
    unsigned int max_atomic_number;
    unsigned int atom_count;

    std::mt19937_64 rng;
    std::normal_distribution<> dist;

    void resetLimits();
    void updateLimits(const Atom &a);

    void processOccupancyList(std::vector<AtomSite> &aList);

    void addAtom(AtomSite a);

    void processAtomList(std::vector<std::string> A, std::vector<double> x, std::vector<double> y, std::vector<double> z, std::vector<double> occ, std::vector<bool> def_u, std::vector<double> ux, std::vector<double> uy, std::vector<double> uz);

public:
    // mostly for opening .xyz files, but will handle .cif
    explicit CrystalStructure(std::string &fPath, CIF::SuperCellInfo info = CIF::SuperCellInfo(), bool fix_cif=false);

    // only for opening cif files
    explicit CrystalStructure(CIF::CIFReader cif, CIF::SuperCellInfo info);

    /// Loads the given xyz file getting the atom coordinates in Angstroms
    /// \param fPath - path to .xyz file to open
    void openXyz(std::string fPath);

    void openCif(std::string fPath, CIF::SuperCellInfo info, bool fix_cif=false);
    void openCif(CIF::CIFReader cif, CIF::SuperCellInfo info);

    std::string fileName() {return file_path;}

    std::vector<AtomSite> atoms() {return atom_list;}

    int atomCountInRange(double xs, double xf, double ys, double yf);

    std::valarray<double> limitsX() {return {min_x, max_x};}

    std::valarray<double> limitsY() {return {min_y, max_y};}

    std::valarray<double> limitsZ() {return {min_z, max_z};}

    unsigned int maxAtomicNumber() {return max_atomic_number;}

    bool thermalFileDefined() { return file_defined_thermals; }

};

#endif // CRYSTALSTRUCTURE_H
