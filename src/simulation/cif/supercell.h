//
// Created by Jon on 29/01/2019.
//

#ifndef CLTEM_SUPERCELL_H
#define CLTEM_SUPERCELL_H

#include <Eigen/Dense>

#include "cifreader.h"

namespace CIF {
    struct SuperCellInfo {
        Eigen::Vector3d uvw;
        Eigen::Vector3d abc;
        Eigen::Vector3d widths;
        Eigen::Vector3d tilts;

        void setZoneAxis(double u, double v, double w) { setUVW(u, v, w); }

        void setUVW(double u, double v, double w) { uvw << u, v, w; }

        void setHorizontalAxis(double a, double b, double c) { setABC(a, b, c); }

        void setABC(double a, double b, double c) { abc << a, b, c; }

        void setWidths(double x, double y, double z) { widths << x, y, z; }

        void setTilts(double alpha, double beta, double gamma) { tilts << alpha, beta, gamma; }
    };

    void makeSuperCell(CIFReader cif, SuperCellInfo info, std::vector<std::string> &A, std::vector<float> &x,
                       std::vector<float> &y, std::vector<float> &z, std::vector<float> &occ);

    void makeSuperCell(CIFReader cif, Eigen::Vector3d uvw, Eigen::Vector3d abc, Eigen::Vector3d widths,
                       Eigen::Vector3d tilts, std::vector<std::string> &A, std::vector<float> &x, std::vector<float> &y,
                       std::vector<float> &z, std::vector<float> &occ);

    void calculateTiling(std::vector<Eigen::Vector3d> &basis, double x_width, double y_width, double z_width,
                         Eigen::Vector3i &mins, Eigen::Vector3i &maxs);

    bool testInRange(Eigen::Vector3d pos, float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);

    template<typename T>
    int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }
}

#endif //CLTEM_SUPERCELL_H
