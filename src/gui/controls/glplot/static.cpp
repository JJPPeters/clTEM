//
// Created by Jon on 03/09/2018.
//

#include "static.h"

namespace PGL {

    Eigen::Matrix4f initTranslationTransform(float x, float y, float z) {
        Eigen::Matrix4f output;
        output << 1.0f, 0.0f, 0.0f, x,
                0.0f, 1.0f, 0.0f, y,
                0.0f, 0.0f, 1.0f, z,
                0.0f, 0.0f, 0.0f, 1.0f;
        return output;
    }

    Eigen::Matrix4f initCameraTransform(const Eigen::Vector3f &Target, const Eigen::Vector3f &Up) {
        Eigen::Vector3f N = Target;
        N.normalize();
        Eigen::Vector3f U = Up;
        U.normalize();
        U = U.cross(N);
        Eigen::Vector3f V = N.cross(U);

        Eigen::Matrix4f output;
        output << U[0], U[1], U[2], 0.0f,
                V[0], V[1], V[2], 0.0f,
                N[0], N[1], N[2], 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f;

        return output;
    }

    Eigen::Matrix4f initScaleTransform(float ScaleX, float ScaleY, float ScaleZ) {
        Eigen::Matrix4f output;
        output << ScaleX, 0.0f,   0.0f,   0.0f,
                0.0f,   ScaleY, 0.0f,   0.0f,
                0.0f,   0.0f,   ScaleZ, 0.0f,
                0.0f,   0.0f,   0.0f,   1.0f;
        return output;
    }

    Eigen::Matrix4f initRotateTransform(float RotateX, float RotateY, float RotateZ, float centreX, float centreY, float centreZ) {
        Eigen::Matrix4f rx, ry, rz;

        const auto x = ToRadian(RotateX);
        const auto y = ToRadian(RotateY);
        const auto z = ToRadian(RotateZ);

        rx << 1.0f, 0.0f,    0.0f,     0.0f,
                0.0f, cosf(x), -sinf(x), 0.0f,
                0.0f, sinf(x), cosf(x) , 0.0f,
                0.0f, 0.0f   , 0.0f    , 1.0f;

        ry << cosf(y),  0.0f, sinf(y),  0.0f,
                0.0f   ,  1.0f, 0.0f    , 0.0f,
                -sinf(y), 0.0f, cosf(y) , 0.0f,
                0.0f   ,  0.0f, 0.0f    , 1.0f;

        rz << cosf(z), -sinf(z), 0.0f, 0.0f,
                sinf(z), cosf(z) , 0.0f, 0.0f,
                0.0f   , 0.0f    , 1.0f, 0.0f,
                0.0f   , 0.0f    , 0.0f, 1.0f;

        auto shift = PGL::initTranslationTransform(centreX, centreY, centreZ);
        auto unshift = PGL::initTranslationTransform(-centreX, -centreY, -centreZ);

        // Brackets here are very important to force the correct order of multiplication
        return unshift * ((rx * ry * rz) * shift);
    }

    Eigen::Matrix4f initPersProjTransform(const PersProjInfo& p) {
        const float ar         = p.Width / p.Height;
        const float zRange     = p.zNear - p.zFar;
        const float tanHalfFOV = tanf(ToRadian(p.FOV / 2.0f));

        Eigen::Matrix4f output;

        output << 1.0f/(tanHalfFOV * ar), 0.0f,            0.0f,                       0.0f,
                0.0f,                   1.0f/tanHalfFOV, 0.0f,                       0.0f,
                0.0f,                   0.0f,            (-p.zNear - p.zFar)/zRange, 2.0f*p.zFar*p.zNear/zRange,
                0.0f,                   0.0f,            1.0f,                       0.0f;

        return output;
    }


    Eigen::Matrix4f initOrthoProjTransform(const OrthoProjInfo& p) {
        float l = p.l;
        float r = p.r;
        float b = p.b;
        float t = p.t;
        float n = p.n;
        float f = p.f;

        Eigen::Matrix4f output;

        output << 2.0f/(r - l), 0.0f,         0.0f,         -(r + l)/(r - l),
                0.0f,         2.0f/(t - b), 0.0f,         -(t + b)/(t - b),
                0.0f,         0.0f,         2.0f/(f - n), -(f + n)/(f - n),
                0.0f,         0.0f,         0.0f,         1.0f;

        return output;
    }

}