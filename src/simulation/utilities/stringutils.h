#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "enums.h"

namespace Utils
{
    // Taken from http://stackoverflow.com/a/6089413
    std::istream& safeGetline(std::istream& is, std::string& t);

    std::string simModeToString(SimulationMode mode);

    template <typename T>
    std::string numToString(T num, int prec = 10, bool isFixed = false)
    {
        std::ostringstream oss;
        oss.precision(prec);
        if(isFixed)
            oss << std::fixed << num;
        else
            oss << num;
        return oss.str();
    }

    std::string resourceToChar(std::string full_directory, std::string fileName);

    std::vector<float> paramsToVector(std::string full_directory, std::string fileName);

    void ccdToDqeNtf(std::string full_directory, std::string fileName, std::string& name, std::vector<float>& dqe_io, std::vector<float>& ntf_io);
}

#endif // STRINGUTILS_H
