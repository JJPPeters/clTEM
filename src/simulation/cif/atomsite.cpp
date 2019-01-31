//
// Created by Jon on 24/08/2015.
//

#include "atomsite.h"

namespace CIF {
    AtomSite::AtomSite(std::vector<Symmetry> symmetryvector, std::string namein, std::string labelin, double xin, double yin, double zin,
                       double occin) {
        element.push_back(namein);
        label.push_back(labelin);
        positions.push_back(std::vector<double>({wrapPosition(xin), wrapPosition(yin), wrapPosition(zin)}));
        occupancy.push_back(occin);

        applySymmetry(symmetryvector);
    }

    void AtomSite::applySymmetry(Symmetry symmetry) {
        double a = wrapPosition(symmetry.getOperation(0).applyOperation(positions[0]));
        double b = wrapPosition(symmetry.getOperation(1).applyOperation(positions[0]));
        double c = wrapPosition(symmetry.getOperation(2).applyOperation(positions[0]));
        std::vector<double> ctemp = {a, b, c};
        if (Utilities::vectorSearch(positions, ctemp) >= positions.size())
            positions.push_back(ctemp);
    }

    void AtomSite::applySymmetry(std::vector<Symmetry> symmetryvector) {
        for (Symmetry s : symmetryvector)
            applySymmetry(s);
    }

    void AtomSite::addAtom(std::string namein, std::string labelin, double occin) {
        element.push_back(namein);
        label.push_back(labelin);
        occupancy.push_back(occin);
    }

    double AtomSite::wrapPosition(double pos) {
        double dud;
        if (pos >= 1.0)
            pos = std::modf(pos, &dud);
        else if (pos < 0.0)
            pos = 1 + std::modf(pos, &dud);
        return pos;
    }
}