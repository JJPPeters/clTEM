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
    // https://stackoverflow.com/a/20446239
    bool stringEndsWith(const std::string &str, const std::string &suffix);

    bool stringBeginsWith(const std::string &str, const std::string &prefix);

    std::vector<std::string> splitStringSpace(const std::string &in);

    std::vector<std::string> splitStringDelimiter(const std::string &in, char delim);

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

    std::string uintToString(unsigned int num, unsigned int width = 0);

    std::string resourceToChar(std::string full_directory, std::string fileName);

//    std::vector<double> paramsToVector(std::string full_directory, std::string fileName, unsigned int &row_count);

    void readParams(std::string full_directory, std::string file_name);

    void ccdToDqeNtf(std::string full_directory, std::string fileName, std::string& name, std::vector<double>& dqe_io, std::vector<double>& ntf_io);
}

#endif // STRINGUTILS_H
