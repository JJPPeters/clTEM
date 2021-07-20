//
// Created by Jon on 29/01/2019.
//

#ifndef CLTEM_SUPERCELL_H
#define CLTEM_SUPERCELL_H

#include <Eigen/Dense>

#include "cifreader.h"

namespace CIF {
    struct SuperCellInfo {
        SuperCellInfo() : uvw({0, 0, 1}), abc({0, 0, 0}), widths({100, 100, 100}), tilts({0, 0, 0}) {}

        Eigen::Vector3d uvw;
        Eigen::Vector3d abc;
        Eigen::Vector3d widths; // Angstrom
        Eigen::Vector3d tilts; // Degrees

        void setZoneAxis(double u, double v, double w) { setUVW(u, v, w); }

        void setUVW(double u, double v, double w) { uvw << u, v, w; }

        void setHorizontalAxis(double a, double b, double c) { setABC(a, b, c); }

        void setABC(double a, double b, double c) { abc << a, b, c; }

        void setWidths(double x, double y, double z) { widths << x, y, z; }

        void setTilts(double alpha, double beta, double gamma) { tilts << alpha, beta, gamma; }

        bool equalUVW(double u, double v, double w) {return uvw(0) == u && uvw(1) == v && uvw(2) == w;}

        bool equalABC(double a, double b, double c) {return abc(0) == a && abc(1) == b && abc(2) == c;}

        bool equalTilts(double alpha, double beta, double gamma) {return tilts(0) == alpha && tilts(1) == beta && tilts(2) == gamma;}
    };

    void makeSuperCell(CIFReader cif, SuperCellInfo info, std::vector<std::string> &A, std::vector<double> &x,
                       std::vector<double> &y, std::vector<double> &z, std::vector<double> &occ, std::vector<bool> &defined_u,
                       std::vector<double> &ux, std::vector<double> &uy, std::vector<double> &uz,
                       Eigen::Vector3d& u1_vec, Eigen::Vector3d& u2_vec, Eigen::Vector3d& u3_vec);

    void makeSuperCell(CIFReader cif, Eigen::Vector3d uvw, Eigen::Vector3d abc, Eigen::Vector3d widths,
                       Eigen::Vector3d tilts, std::vector<std::string> &A, std::vector<double> &x, std::vector<double> &y,
                       std::vector<double> &z, std::vector<double> &occ, std::vector<bool> &defined_u,
                       std::vector<double> &ux, std::vector<double> &uy, std::vector<double> &uz,
                       Eigen::Vector3d& u1_vec, Eigen::Vector3d& u2_vec, Eigen::Vector3d& u3_vec);

    void calculateTiling(std::vector<Eigen::Vector3d> &basis, double x_width, double y_width, double z_width,
                         Eigen::Vector3i &mins, Eigen::Vector3i &maxs);

    bool testInRange(Eigen::Vector3d pos, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);

    template<typename T>
    int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }
}

#endif //CLTEM_SUPERCELL_H
