//
// Created by Jon on 24/08/2015.
//

#ifndef XYZ_ATOMSITE_H
#define XYZ_ATOMSITE_H

#include <string>

#include "cifutilities.h"
#include "symmetryoperation.h"

namespace CIF {
    class AtomSite {
    public:
        AtomSite() {}

        AtomSite(std::vector<Symmetry> symmetryvector, std::string namein, std::string labelin, double xin, double yin, double zin,
                 double occin = 1.0);

        void applySymmetry(Symmetry symmetry);

        void applySymmetry(std::vector<Symmetry> symmetryvector);

        void addAtom(std::string namein, std::string labelin, double occin);

        bool setIsoU(std::string lbl, double u_iso);

        bool setU(std::string lbl, double u, int i);

        std::vector<double> getOccupancies() { return occupancy; }

        std::vector<std::vector<double>> getPositions() { return positions; }

        std::vector<std::string> getElements() { return element; }

        std::vector<Eigen::Vector3d> getThermals() {return thermal_u;}

    private:
        std::vector<double> occupancy;
        std::vector<Eigen::Vector3d> thermal_u;
        std::vector<std::vector<double>> positions;
        std::vector<std::string> element, label;

        double wrapPosition(double pos);

    };
}

#endif //XYZ_ATOMSITE_H
