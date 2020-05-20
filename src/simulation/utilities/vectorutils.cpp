//
// Created by Jon on 15/05/2018.
//

#include "vectorutils.h"

namespace Utils {
    Eigen::Matrix3d generateRotationAroundVector(Eigen::Vector3d ax, double angle) {
        // normalize otherwise our rotation will modify the magnitude?
        ax.normalize();

        Eigen::Matrix3d c;
        c << 0.0, -ax(2), ax(1),
                ax(2), 0.0, -ax(0),
                -ax(1), ax(0), 0.0;

        return Eigen::Matrix3d::Identity() + c * std::sin(angle) + c * c * (1 - std::cos(angle));
    }

    // theta is about y, phi is about z
    void rotateVectorSpherical(Eigen::Vector3d& z, Eigen::Vector3d& y, double theta, double phi)
    {
        // generate rotation matrix around y
        Eigen::Matrix3d r_theta = generateRotationAroundVector(y, theta);
        Eigen::Matrix3d r_phi = generateRotationAroundVector(z, phi);

        z = r_theta * z;

        z = r_phi * z;
        y = r_phi * y;
    }
}