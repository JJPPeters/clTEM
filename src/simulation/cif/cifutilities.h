//
// Created by Jon on 23/08/2015.
//

#ifndef XYZ_UTILITIES_H
#define XYZ_UTILITIES_H

#include <string>
#include <vector>
#include <sstream>

#include <regex>

#include <cassert>
#include <iostream>

#include <Eigen/Dense>

namespace CIF::Utilities {

        extern const std::vector<std::string> AcceptedAtoms;

        bool isAcceptedAtom(const std::string& symbol);

        std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);

        std::vector<std::string> split(const std::string &s, char delim);

        // https://stackoverflow.com/a/49201823
        std::vector<std::string> split(const std::string &s, std::string delims);

        double stod(std::string& s);

        // https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
        bool string_ends_with(std::string fullString, std::string ending);

        template<typename T>
        int vectorSearch(std::vector<T> vec, T value) {
            int pos = std::find(vec.begin(), vec.end(), value) - vec.begin();

            return pos;
        }

        double regexFindDoubleTag(std::string input, std::string pattern);

        template<typename T>
        Eigen::Matrix3d generateNormalisedRotationMatrix(Eigen::Vector3d A, Eigen::Vector3d B) {
            // https://stackoverflow.com/questions/45142959/calculate-rotation-matrix-to-align-two-vectors-in-3d-space
            A.normalize();
            B.normalize();
            Eigen::Vector3d v = A.cross(B);
            double c = A.dot(B);
            // norm() gets the magnitude (normalize() normalises the vector)
            double s = v.norm();

            // Row major
            // std::vector<T> data = {0.0, -v(2), v(1), v(2), 0.0, -v(0), -v(1), v(0), 0.0};
            // Column maor
            std::vector<T> data = {0.0, v(2), -v(1), -v(2), 0.0, v(0), v(1), -v(0), 0.0};
            Eigen::Matrix3d k(data.data());

            // k * k is proper matrix multiplication
            Eigen::Matrix3d r = Eigen::Matrix3d::Identity() + k + k * k * (1 - c) / (s * s);
//            Eigen::Matrix3d r = Eigen::Matrix3d::Identity() + k + k * k / (1 + c);
//            r.normalize();
            return r;
        }

        template<typename T>
        Eigen::Matrix3d generateRotationMatrix(Eigen::Vector3d axis, double theta) {
            axis = axis / std::sqrt(axis.dot(axis));
            double a = std::cos(theta / 2);
            auto bcd = -1 * axis * std::sin(theta / 2);
            auto aa = a * a;
            auto bb = bcd(0) * bcd(0);
            auto cc = bcd(1) * bcd(1);
            auto dd = bcd(2) * bcd(2);
            auto ab = a * bcd(0);
            auto ac = a * bcd(1);
            auto ad = a * bcd(2);
            auto bc = bcd(0) * bcd(1);
            auto bd = bcd(0) * bcd(2);
            auto cd = bcd(1) * bcd(2);

            // row-major!
//            std::vector<T> data = {aa + bb - cc - dd, 2 * (bc + ad), 2 * (bd - ac), 2 * (bc - ad), aa + cc - bb - dd,
//                                   2 * (cd + ab), 2 * (bd + ac), 2 * (cd - ab), aa + dd - bb - cc};

            // Column major!
            std::vector<T> data = {aa + bb - cc - dd, 2 * (bc - ad), 2 * (bd + ac),
                                   2 * (bc + ad), aa + cc - bb - dd, 2 * (cd - ab),
                                   2 * (bd - ac), 2 * (cd + ab), aa + dd - bb - cc};

            return Eigen::Matrix3d(data.data());
        }

        /// Removes CIF comments and whitespace at start or end of line
        std::string stripCommentsWhitespace(const std::string& input);

    }
#endif //XYZ_UTILITIES_H
