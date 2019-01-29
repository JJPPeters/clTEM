//
// Created by Jon on 29/01/2019.
//

#include "supercell.h"

void makeSuperCell(CIFReader cif, Eigen::Vector3d uvw, Eigen::Vector3d abc, Eigen::Vector3d widths, Eigen::Vector3d tilts) {
    // TODO: check that the uvw and abc vectors are no colinear

    UnitCell  cell = cif.getUnitCell();

    auto geom  = cell.getCellGeometry();
    auto a_vector = Eigen::Vector3d(geom.getAVector().data());
    auto b_vector = Eigen::Vector3d(geom.getBVector().data());
    auto c_vector = Eigen::Vector3d(geom.getCVector().data());

    // TODO: try to use more eigen matrices/vectors instead of stl
//    Eigen::Matrix3d basis;
//    basis << // use std::Vectors here?

    // for easy loopage
    // TODO: make this an eigen vector of vectors
    std::vector<Eigen::Vector3d> basis = {a_vector, b_vector, c_vector};

    // this gives us the element type, occupancy and FRACTIONAL coords as well as the basis vectors
    // now we just need rotate them and tile them to fill the space

    // create vector from the zone axis we want
    Eigen::Vector3d uvw_vec = uvw(0) * a_vector + uvw(1) * b_vector + uvw(2) * c_vector;
    // create matrix with direction we want to map onto (here is is the z direction)
    Eigen::Vector3d z_direction(std::vector<double>({0.0, 0.0, 1.0}).data());
    // create the rotation matrix
    auto za_rotation = Utilities::generateNormalisedRotationMatrix<double>(uvw_vec, z_direction);

    if (za_rotation.array().isNaN().sum() != 0) {
        throw std::runtime_error("Zone axis rotation matrix contains NaNs");
    }

    // similarly for rotation in x,y plane, though here we default to 0 rotation
    Eigen::Matrix3d xy_rotation = Eigen::Matrix3d::Identity();
    if ((abc.array() != 0).any()) {
        Eigen::Vector3d abc_vec = abc(0) * a_vector + abc(1) * b_vector + abc(2) * c_vector;
        abc_vec = za_rotation * abc;
        abc_vec(2) = 0.0; // no rotate the z-axis!
        abc.normalize();
        Eigen::Vector3d x_direction(std::vector<double>({1.0, 0.0, 0.0}).data()); // TODO: use << initialisation
        // Need the negative angle here? so give inputs in opposite order?
        xy_rotation = Utilities::generateNormalisedRotationMatrix<double>(x_direction, abc);
    }

    if (xy_rotation.array().isNaN().sum() != 0) {
        throw std::runtime_error("Normal rotation matrix contains NaNs");
    }

    // convert angles to radians
    double alpha = tilts(0) * M_PI / 180;
    double beta = tilts(1) * M_PI / 180;
    double gamma = tilts(2) * M_PI / 180;

    // generate the small rotation matrices
    auto x_rotation = Utilities::generateRotationMatrix<double>({1.0, 0.0, 0.0}, alpha);
    auto y_rotation = Utilities::generateRotationMatrix<double>({0.0, 1.0, 0.0}, beta);
    auto z_rotation = Utilities::generateRotationMatrix<double>({0.0, 0.0, 1.0}, gamma);

    for (int i = 0; i < basis.size(); ++i) {
        basis[i] = za_rotation * basis[i];
        basis[i] = xy_rotation * basis[i];
        basis[i] = x_rotation * basis[i];
        basis[i] = y_rotation * basis[i];
        basis[i] = z_rotation * basis[i];
    }

    auto range = calculateTiling(basis, widths(0), widths(1), widths(2));

    // get total number of atoms in our xyz file
    int count = 0;
    for (auto at : cell.getAtoms())
        count += at.getElements().size() * at.getPositions().size();
    count = count * range(0) * range(1) * range(2);

    if ((range.array() == 0).any())
        throw std::runtime_error("Did not find any atoms inside limits");

    std::vector<std::string> element_list(count);
    std::vector<double> x_list(count);
    std::vector<double> y_list(count);
    std::vector<double> z_list(count);
    std::vector<double> occupancy_list(count);
    int it = 0;

    for (auto at : cell.getAtoms()) {
        for (auto pos : at.getPositions()) {
            // convert from fractional coordinates
            auto p = pos[0] * basis[0] + pos[1] * basis[1] + pos[2] * basis[2];
            for (int k = 0; k < range(2); ++k) {
                auto k_factor = basis[2] * k;
                for (int j = 0; j < range(1); ++j) {
                    auto j_factor = basis[1] * j;
                    for (int i = 0; i < range(0); ++i) {
                        auto i_factor = basis[0] * i;
                        auto new_pos = p + i_factor + j_factor + k_factor;

                        for (int ind = 0; ind < at.getElements().size(); ++ind) {
                            element_list[it] = at.getElements()[ind];
                            x_list[it] = new_pos(0);
                            y_list[it] = new_pos(1);
                            z_list[it] = new_pos(2);
                            occupancy_list[it] = at.getOccupancies()[ind];
                            ++it;

                            // TODO: process occupancies (can there be overlap with the .xyz functions
                        }
                    }
                }
            }
        }
    }

    if (x_list.empty() || y_list.empty() || z_list.empty())
        throw std::runtime_error("Did not find any atoms inside limits");





    // TODO: all this can be put into a function
    // re use the min and max variables here
    double x_min = *std::min_element(x_list.begin(), x_list.end());
    double x_max = *std::max_element(x_list.begin(), x_list.end());
    double y_min = *std::min_element(y_list.begin(), y_list.end());
    double y_max = *std::max_element(y_list.begin(), y_list.end());
    double z_min = *std::min_element(z_list.begin(), z_list.end());
    double z_max = *std::max_element(z_list.begin(), z_list.end());
    // TODO: find mid point of new crystal
    // TODO: find crop limits

    double x_mid = (x_min + x_max) / 2;
    double y_mid = (y_min + y_max) / 2;
    double z_mid = (z_min + z_max) / 2;

    double x_low = x_mid - (widths(0) / 2);
    double y_low = y_mid - (widths(1) / 2);
    double z_low = z_mid - (widths(2) / 2);

    double x_high = x_mid + (widths(0) / 2);
    double y_high = y_mid + (widths(1) / 2);
    double z_high = z_mid + (widths(2) / 2);

    std::vector<bool> position_valid(count, false);
    int valid_count = 0;

    // find valid positions
    for (int i = 0; i < count; ++i)
        if (x_list[i] > x_low && x_list[i] < x_high)
            if (y_list[i] > y_low && y_list[i] < y_high)
                if (z_list[i] > z_low && z_list[i] < z_high) {
                    position_valid[i] = true;
                    valid_count++;
                }

    // TODO: Do I want to make my list of Atoms here

    // TODO: how to return data
}


Eigen::Vector3i calculateTiling(std::vector<Eigen::Vector3d> &basis, double x_width, double y_width, double z_width) {
    Eigen::MatrixXd affine_basis(3, 4);
    affine_basis << 0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0;

    Eigen::Matrix4d real_basis;
    real_basis << 1.0, 1.0, 1.0, 1.0,
            0.0, basis[0](0), basis[1](0), basis[2](0),
            0.0, basis[0](1), basis[1](1), basis[2](1),
            0.0, basis[0](2), basis[1](2), basis[2](2);

    auto affine_transform_matrix = affine_basis * real_basis.inverse();

    Eigen::MatrixXd cuboid(8, 4);
    cuboid << 1.0, 0.0, 0.0, 0.0,
            1.0, x_width, 0.0, 0.0,
            1.0, 0.0, y_width, 0.0,
            1.0, 0.0, 0.0, z_width,
            1.0, x_width, y_width, 0,
            1.0, x_width, 0.0, z_width,
            1.0, 0.0, y_width, z_width,
            1.0, x_width, y_width, z_width;

    // affine transform the cuboid
    Eigen::MatrixXd affine_cube(8, 3);
    for (int i = 0; i < cuboid.rows(); ++i) {
        Eigen::Vector4d temp_row = cuboid.row(i);
        affine_cube.row(i) = affine_transform_matrix * temp_row;
    }

    // get the limits of the cube in affine space
    double x_max = std::numeric_limits<double>::min();
    double x_min = std::numeric_limits<double>::max();
    double y_max = std::numeric_limits<double>::min();
    double y_min = std::numeric_limits<double>::max();
    double z_max = std::numeric_limits<double>::min();
    double z_min = std::numeric_limits<double>::max();

    for (int i = 0; i < affine_cube.rows(); ++i) {
        if (affine_cube(i, 0) > x_max)
            x_max = affine_cube(i, 0);
        if (affine_cube(i, 0) < x_min)
            x_min = affine_cube(i, 0);
        if (affine_cube(i, 1) > y_max)
            y_max = affine_cube(i, 1);
        if (affine_cube(i, 1) < y_min)
            y_min = affine_cube(i, 1);
        if (affine_cube(i, 2) > z_max)
            z_max = affine_cube(i, 2);
        if (affine_cube(i, 2) < z_min)
            z_min = affine_cube(i, 2);
    }

    // get integer cell range (rounded up to nearest even number)
    int x_range = static_cast<int>(std::ceil(x_max - x_min));
    if (x_range % 2 != 0)
        x_range += 1;
    int y_range = static_cast<int>(std::ceil(y_max - y_min));
    if (y_range % 2 != 0)
        y_range += 1;
    int z_range = static_cast<int>(std::ceil(z_max - z_min));
    if (z_range % 2 != 0)
        z_range += 1;

    Eigen::Vector3d ranges;
    ranges << x_range, y_range, z_range;

    return ranges;
}