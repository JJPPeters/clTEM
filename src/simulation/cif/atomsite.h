//
// Created by Jon on 24/08/2015.
//

#ifndef XYZ_ATOMSITE_H
#define XYZ_ATOMSITE_H

#include <string>

#include "cifutilities.h"
#include "symmetryoperation.h"

class AtomSite
{
public:
    AtomSite(){}

    AtomSite(std::vector<Symmetry> symmetryvector, std::string namein, double xin, double yin, double zin, double occin = 1.0);

    void applySymmetry(Symmetry symmetry);

    void applySymmetry(std::vector<Symmetry> symmetryvector);

    void addAtom(std::string namein, double occin);

    std::vector<double> getOccupancies(){return occupancy;}
    std::vector<std::vector<double>> getPositions(){return positions;}
    std::vector<std::string> getElements(){return name;}

private:
    std::vector<double> occupancy;
    std::vector<std::vector<double>> positions;
    std::vector<std::string> name;

    double wrapPosition(double pos);

};


#endif //XYZ_ATOMSITE_H
