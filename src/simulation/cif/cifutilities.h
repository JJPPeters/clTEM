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

            if (A.isApprox(B))
                return Eigen::Matrix3d::Identity();

            Eigen::Vector3d v = A.cross(B);
            if (A.isApprox((-B))) {
                Eigen::Vector3d temp(1.0, 0.0, 0.0);
                if (temp.isApprox(A))
                    temp(2) = 1.0;

                v = A.cross(temp);
            }
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
            return r;
        }

        template<typename T>
        Eigen::Matrix3d generateRotationMatrix(Eigen::Vector3d ax, double theta) {
            // https://computergraphics.stackexchange.com/a/2404
            // normalize otherwise our rotation will modify the magnitude?
            ax.normalize();

            Eigen::Matrix3d c;
            c << 0.0, -ax(2), ax(1),
                    ax(2), 0.0, -ax(0),
                    -ax(1), ax(0), 0.0;

            return Eigen::Matrix3d::Identity() + c * std::sin(theta) + c * c * (1 - std::cos(theta));
        }

        /// Removes CIF comments and whitespace at start or end of line
        std::string stripCommentsWhitespace(const std::string& input);

    }
#endif //XYZ_UTILITIES_H
