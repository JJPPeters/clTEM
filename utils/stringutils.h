//
// Created by jon on 09/12/17.
//

#ifndef CLTEM_GUI_STRINGUTILS_H
#define CLTEM_GUI_STRINGUTILS_H

#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <QApplication>

#include "utilities/stringutils.h"

namespace Utils
{
    static std::string resourceToChar(std::string folder, std::string resourcePath)
    {
        std::string exe_path = QApplication::instance()->applicationDirPath().toStdString();
        std::ifstream inStream(exe_path + "/" + folder + "/" + resourcePath);

        if (inStream.fail())
            throw std::runtime_error("Error opening resource file: " + resourcePath);

        std::string fileContents((std::istreambuf_iterator<char>(inStream)), (std::istreambuf_iterator<char>()));

        inStream.close();

        return fileContents;
    }

    static std::string kernelToChar(std::string kernelName)
    {
        return Utils::resourceToChar("kernels", kernelName);
    }

    static std::vector<float> paramsToVector(std::string paramsName, std::string folder = "params")
    {
        std::string exe_path = QApplication::instance()->applicationDirPath().toStdString();
//        return utils::resourceToChar("params", paramsName);
        std::ifstream inStream(exe_path + "/" + folder + "/" + paramsName);
        std::vector<float> out;
        float p;

        while (inStream >> p)
            out.push_back(p);

        inStream.close();

        return out;
    }

    template <typename T>
    static QString numToQString(T num, int prec = 10, bool isFixed = false) {
        return QString::fromStdString(Utils::numToString(num, prec, isFixed));
    }
};


#endif //CLTEM_GUI_STRINGUTILS_H
