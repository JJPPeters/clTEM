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

namespace CIF {
    static struct Utilities {


        static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delim)) {
                elems.push_back(item);
            }
            return elems;
        }

        static std::vector<std::string> split(const std::string &s, char delim) {
            std::vector<std::string> elems;
            split(s, delim, elems);
            return elems;
        }

        static double stod(std::string& s) {
            if (s == ".")
                return 0.0;

            return std::stod(s);
        }

        template<typename T>
        static int vectorSearch(std::vector<T> vec, T value) {
            int pos = std::find(vec.begin(), vec.end(), value) - vec.begin();

//        if (pos >= vec.size())
//            return 0; // TODO: throw an error here
//        else
            return pos;
        }

        static double regexFindDoubleTag(std::string input, std::string pattern) {
            std::regex rgx(pattern);
            std::smatch match;

            if (!std::regex_search(input, match, rgx))
                return 0.0; // TODO: throw error

            return std::stod(std::string(match[1].str()));
        }

        template<typename T>
        static Eigen::Matrix3d generateNormalisedRotationMatrix(const Eigen::Vector3d &A, const Eigen::Vector3d &B) {
            Eigen::Vector3d wm = A.cross(B);

            wm.normalize(); // I'm assuming that Eigen will handle the case of a zero vector...

            std::vector<T> data = {0.0, -wm(2), wm(1), wm(2), 0.0, -wm(0), -wm(1), wm(0), 0.0};
            Eigen::Matrix3d w_hat(data.data());

            double cos_tht = A.dot(B) / (A.norm() * B.norm());

            double tht = std::acos(cos_tht);

            // This is the rotation matrix we want
            return Eigen::Matrix3d::Identity() + w_hat * std::sin(tht) + w_hat * w_hat * (1 - cos_tht);
        }

        template<typename T>
        static Eigen::Matrix3d generateRotationMatrix(Eigen::Vector3d axis, double theta) {
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

            std::vector<T> data = {aa + bb - cc - dd, 2 * (bc + ad), 2 * (bd - ac), 2 * (bc - ad), aa + cc - bb - dd,
                                   2 * (cd + ab), 2 * (bd + ac), 2 * (cd - ab), aa + dd - bb - cc};
            return Eigen::Matrix3d(data.data());
        }

        static std::string stripComments(const std::string& input) {

            std::string output = "";
            std::istringstream iss(input);
            for (std::string line; std::getline(iss, line); ) {
                size_t comm = line.rfind('#');
                std::string temp = line.substr(0, comm);
                temp.erase(temp.find_last_not_of(R"( \n\r\t)")+1); // I don't really care about whitespace, but I don't want blank lines (i.e. only whitespace)

                if (!temp.empty())
                    output += line.substr(0, comm) + "\n";
            }

            return output;
        }

    } Utilities;
}
#endif //XYZ_UTILITIES_H
