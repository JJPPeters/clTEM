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
        thermal_u.emplace_back();
        thermal_defined = false;

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
        thermal_u.emplace_back();
    }

    double AtomSite::wrapPosition(double pos) {
        double dud;
        if (pos >= 1.0)
            pos = std::modf(pos, &dud);
        else if (pos < 0.0)
            pos = 1 + std::modf(pos, &dud);
        return pos;
    }

    bool AtomSite::setIsoU(std::string lbl, double u_iso) {

        long long int i = std::find(label.begin(), label.end(), lbl) - label.begin();

        if (i >= label.size())
            return false;

        thermal_u[i](0) = u_iso;
        thermal_u[i](1) = u_iso;
        thermal_u[i](2) = u_iso;
        thermal_defined = true;
        return true;
    }

    bool AtomSite::setU(std::string lbl, double u, int i) {
        long long int ind = std::find(label.begin(), label.end(), lbl) - label.begin();

        if (ind >= label.size())
            return false;

        thermal_u[ind](i) = u;
        thermal_defined = true;
        return true;
    }


}