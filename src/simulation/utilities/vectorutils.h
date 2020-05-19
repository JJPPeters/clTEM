//
// Created by Jon on 15/05/2018.
//

#ifndef CLTEM_VECTORUTILS_H
#define CLTEM_VECTORUTILS_H


#include <vector>
#include <algorithm>
#include <stddef.h>
#include <valarray>

#include <Eigen/Dense>

namespace Utils
{
    template <typename T>
    int findItemIndex(std::vector<T>& vec, T val)
    {
        ptrdiff_t pos = std::distance(vec.begin(), std::find(vec.begin(), vec.end(), val));
        if(pos >= vec.size()) {
            return -1;
        }

        return static_cast<int>(pos);
    }

    // https://stackoverflow.com/a/1041939
    template <typename T>
    std::vector<T> removeDuplicates(std::vector<T>& in)
    {
        auto vec = in;

        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());

        return vec;
    }

    Eigen::Matrix3d generateRotationAroundVector(Eigen::Vector3d ax, double angle);

    // theta is about y, phi is about z
    void rotateVectorSpherical(Eigen::Vector3d& z, Eigen::Vector3d& y, double theta, double phi);

}


#endif //CLTEM_VECTORUTILS_H
