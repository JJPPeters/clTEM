#ifndef CRYSTALSTRUCTURE_H
#define CRYSTALSTRUCTURE_H

#include <vector>
#include <string>
#include <valarray>
#include <random>

#include "clwrapper.h"
#include "clstatic.h"
#include "utilities/commonstructs.h"

#include "thermalvibrations.h"

#include "atom.h"

class CrystalStructure
{
private:
    /// Vector of atoms with coordinates in Angstroms
    std::vector<AtomSite> Atoms;

    std::vector<int> AtomTypes;

    std::string filePath;

    bool file_defined_thermals;

    float ScaleFactor;

    float MaxX;
    float MinX;
    float MaxY;
    float MinY;
    float MaxZ;
    float MinZ;

    unsigned int MaxAtomicNumber; // This is only really used to see if it is available in our parameterisation
    unsigned int AtomCount;

    std::mt19937 rng;
    std::uniform_real_distribution<> dist;

    void resetLimits();
    void updateLimits(const Atom &a);

    void processOccupancyList(std::vector<AtomSite> &aList);

    void addAtom(AtomSite a);

public:
    explicit CrystalStructure(std::string& fPath);

    std::vector<int> getAtomsTypes() {return AtomTypes;}

    bool isThermalFileDefined() { return file_defined_thermals; }

    /// Loads the given xyz file getting the atom coordinates in Angstroms
    /// \param fPath - path to .xyz file to open
    void openXyz(std::string fPath);

    std::string getFileName() {return filePath;}

    std::vector<AtomSite> getAtoms() {return Atoms;}

    int getAtomCountInRange(float xs, float xf, float ys, float yf);

    std::valarray<float> getLimitsX() {return {MinX, MaxX};}

    std::valarray<float> getLimitsY() {return {MinY, MaxY};}

    std::valarray<float> getLimitsZ() {return {MinZ, MaxZ};}

    unsigned int getMaxAtomicNumber() {return MaxAtomicNumber;}
};

#endif // CRYSTALSTRUCTURE_H
