#ifndef CRYSTALSTRUCTURE_H
#define CRYSTALSTRUCTURE_H

#include <vector>
#include <string>
#include <valarray>
#include <random>

#include "clwrapper.h"
#include "clstatic.h"
#include "utilities/commonstructs.h"

#include "atom.h"

class CrystalStructure
{
private:
    /// Vector of atoms with coordinates in Angstroms
    std::vector<Atom> Atoms;

    std::string filePath;

    float ScaleFactor;

    float MaxX;
    float MinX;
    float MaxY;
    float MinY;
    float MaxZ;
    float MinZ;

    std::mt19937 rng;
    std::normal_distribution<> dist;

    int AtomCount;

    void resetLimits();
    void updateLimits(const Atom &a);

    void processOccupancyList(std::vector<AtomSite> &aList);

public:
    CrystalStructure(std::string& fPath);

    /// Loads the given xyz file getting the atom coordinates in Angstroms
    /// \param fPath - path to .xyz file to open
    void openXyz(std::string fPath);

    std::string getFileName() {return filePath;}

    std::vector<Atom> getAtoms() {return Atoms;}

    float generateTdsFactor();

    void clearStructure();

    float getZRange();

//    std::tuple<float, float> getSimRanges();
    std::tuple<float, float> getStructRanges();

    int getTotalAtomCount() {return AtomCount;}
    int getAtomCountInRange(float xs, float xf, float ys, float yf);

//    float getStructMinX() {return MinX;}
//    float getStructMaxX() {return MaxX;}
    std::valarray<float> getLimitsX() {return {MinX, MaxX};}

//    float getStructMinY() {return MinY;}
//    float getStructMaxY() {return MaxY;}
    std::valarray<float> getLimitsY() {return {MinY, MaxY};}

//    float getStructMinZ() {return MinZ;}
//    float getStructMaxZ() {return MaxZ;}
    std::valarray<float> getLimitsZ() {return {MinZ, MaxZ};}
};

#endif // CRYSTALSTRUCTURE_H
