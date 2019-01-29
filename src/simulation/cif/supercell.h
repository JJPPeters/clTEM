//
// Created by Jon on 29/01/2019.
//

#ifndef CLTEM_SUPERCELL_H
#define CLTEM_SUPERCELL_H

#include <Eigen/Dense>

#include "cifreader.h"

void makeSuperCell(CIFReader cif, Eigen::Vector3d uvw, Eigen::Vector3d abc, Eigen::Vector3d widths, Eigen::Vector3d tilts);

Eigen::Vector3i calculateTiling(std::vector<Eigen::Vector3d> &basis, double x_width, double y_width, double z_width);

#endif //CLTEM_SUPERCELL_H
