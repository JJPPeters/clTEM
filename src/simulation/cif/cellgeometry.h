//
// Created by Jon on 25/08/2015.
//

#ifndef XYZ_CELLGEOMETRY_H
#define XYZ_CELLGEOMETRY_H

#include <vector>

#include "cifutilities.h"
namespace CIF {
    class CellGeometry {
    public:
        CellGeometry() {}

        CellGeometry(double ain, double bin, double cin, double alphain, double betain, double gammain);

        std::vector<double> getAVector() { return avec; }

        std::vector<double> getBVector() { return bvec; }

        std::vector<double> getCVector() { return cvec; }

    private:
        double a, b, c, alpha, beta, gamma;

        std::vector<double> avec, bvec, cvec;

        void calculateCartesianBasis();
    };
}

#endif //XYZ_CELLGEOMETRY_H
