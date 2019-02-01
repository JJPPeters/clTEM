//
// Created by Jon on 29/01/2019.
//

#include "supercell.h"

namespace CIF {

    void makeSuperCell(CIFReader cif, SuperCellInfo info, std::vector<std::string> &A, std::vector<float> &x,
                       std::vector<float> &y, std::vector<float> &z, std::vector<float> &occ, std::vector<float> &ux,
                       std::vector<float> &uy, std::vector<float> &uz) {
        makeSuperCell(cif, info.uvw, info.abc, info.widths, info.tilts, A, x, y, z, occ, ux, uy, uz);
    }

    void makeSuperCell(CIFReader cif, Eigen::Vector3d uvw, Eigen::Vector3d abc, Eigen::Vector3d widths,
                       Eigen::Vector3d tilts, std::vector<std::string> &A, std::vector<float> &x, std::vector<float> &y,
                       std::vector<float> &z, std::vector<float> &occ, std::vector<float> &ux, std::vector<float> &uy,
                       std::vector<float> &uz) {
        // TODO: check that the uvw and abc vectors are no colinear
        UnitCell cell = cif.getUnitCell();

        auto geom = cell.getCellGeometry();
        auto a_vector = Eigen::Vector3d(geom.getAVector().data());
        auto b_vector = Eigen::Vector3d(geom.getBVector().data());
        auto c_vector = Eigen::Vector3d(geom.getCVector().data());

        // TODO: try to use more eigen matrices/vectors instead of stl
//    Eigen::Matrix3d basis;
//    basis << // use std::Vectors here?

        // for easy loopage
        // TODO: make this an eigen vector of eigen vectors
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
            abc_vec.normalize();
            Eigen::Vector3d x_direction(std::vector<double>({1.0, 0.0, 0.0}).data()); // TODO: use << initialisation
            // Need the negative angle here? so give inputs in opposite order?
            xy_rotation = Utilities::generateNormalisedRotationMatrix<double>(x_direction, abc_vec);
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

        Eigen::Vector3i mins, maxs;
        calculateTiling(basis, widths(0), widths(1), widths(2), mins, maxs);
        Eigen::Vector3i range = maxs - mins;

        // get total number of atoms in our xyz file
        int count = 0;
        for (auto at : cell.getAtoms())
            count += at.getElements().size() * at.getPositions().size();
        count = count * range(0) * range(1) * range(2);

        if ((range.array() == 0).any())
            throw std::runtime_error("Did not find any atoms inside limits");

        A.resize(count);
        x.resize(count);
        y.resize(count);
        z.resize(count);
        occ.resize(count);
        ux.resize(count);
        uy.resize(count);
        uz.resize(count);

        int it = 0;

        for (auto at : cell.getAtoms()) {
            for (auto pos : at.getPositions()) {
                // convert from fractional coordinates
                auto p = pos[0] * basis[0] + pos[1] * basis[1] + pos[2] * basis[2];

                for (int k = mins(2); k < maxs(2); ++k) {
                    auto k_factor = basis[2] * k;

                    for (int j = mins(1); j < maxs(1); ++j) {
                        auto j_factor = basis[1] * j;

                        for (int i = mins(0); i < maxs(0); ++i) {
                            auto i_factor = basis[0] * i;

                            auto new_pos = p + i_factor + j_factor + k_factor;

                            for (int ind = 0; ind < at.getElements().size(); ++ind) {

                                if (testInRange(new_pos, 0.0, widths(0), 0.0, widths(1), 0.0, widths(2))) {
                                    A[it] = at.getElements()[ind];
                                    x[it] = new_pos(0);
                                    y[it] = new_pos(1);
                                    z[it] = new_pos(2);
                                    occ[it] = at.getOccupancies()[ind];
                                    ux[it] = at.getThermals()[ind](0);
                                    uy[it] = at.getThermals()[ind](1);
                                    uz[it] = at.getThermals()[ind](2);

                                    ++it;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (x.empty() || y.empty() || z.empty())
            throw std::runtime_error("Did not find any atoms inside limits");

        // resize back down to our valid entries
        A.resize(it);
        x.resize(it);
        y.resize(it);
        z.resize(it);
        occ.resize(it);
        ux.resize(it);
        uy.resize(it);
        uz.resize(it);
    }


    void calculateTiling(std::vector<Eigen::Vector3d> &basis, double x_width, double y_width, double z_width,
                         Eigen::Vector3i &mins, Eigen::Vector3i &maxs) {
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
        long max = std::numeric_limits<long>::min();
        long min = std::numeric_limits<long>::max();

        maxs << max, max, max;
        mins << min, min, min;

        for (int i = 0; i < affine_cube.rows(); ++i) {
            long rnd_x = (long) sgn(affine_cube(i, 0)) * std::ceil(std::abs(affine_cube(i, 0)));
            long rnd_y = (long) sgn(affine_cube(i, 1)) * std::ceil(std::abs(affine_cube(i, 1)));
            long rnd_z = (long) sgn(affine_cube(i, 2)) * std::ceil(std::abs(affine_cube(i, 2)));

            if (rnd_x > maxs(0))
                maxs(0) = rnd_x;
            if (rnd_x < mins(0))
                mins(0) = rnd_x;
            if (rnd_y > maxs(1))
                maxs(1) = rnd_y;
            if (rnd_y < mins(1))
                mins(1) = rnd_y;
            if (rnd_z > maxs(2))
                maxs(2) = rnd_z;
            if (rnd_z < mins(2))
                mins(2) = rnd_z;
        }
    }

    bool testInRange(Eigen::Vector3d pos, float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) {
        return pos(0) >= xmin && pos(0) <= xmax && pos(1) >= ymin && pos(1) <= ymax && pos(2) >= zmin && pos(2) <= zmax;
    }

}