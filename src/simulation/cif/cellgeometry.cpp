//
// Created by Jon on 25/08/2015.
//

#include "cellgeometry.h"
namespace CIF {
    CellGeometry::CellGeometry(double ain, double bin, double cin, double alphain, double betain, double gammain) {
        a = ain;
        b = bin;
        c = cin;
        alpha = alphain;
        beta = betain;
        gamma = gammain;

        calculateCartesianBasis();
    }

    void CellGeometry::calculateCartesianBasis() {
        // !! convert to radians
        double arad = alpha * (M_PI / 180);
        double brad = beta * (M_PI / 180);
        double grad = gamma * (M_PI / 180);

        // easy enough
        avec = {a, 0.0, 0.0};
        // slightly trickier
        bvec = {b * std::cos(grad), b * std::sin(grad), 0.0};
        // WOAH
        double cv1 = c * std::cos(brad);
        double cv2 = c * (std::cos(arad) - std::cos(brad) * std::cos(grad)) / std::sin(grad);
        double cv3 =
                c * std::sqrt((std::cos(brad - grad) - std::cos(arad)) * (std::cos(arad) - std::cos(brad + grad))) /
                std::sin(grad);

        cvec = {cv1, cv2, cv3};
    }
}