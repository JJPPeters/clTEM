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

namespace Utils_Qt
{
    static std::string kernelToChar(std::string kernelName)
    {
        std::string full_path = QApplication::instance()->applicationDirPath().toStdString() + "/kernels";
        return Utils::resourceToChar(full_path, kernelName);
    }

    static std::vector<float> paramsToVector(std::string paramsName, std::string folder = "params")
    {
        std::string exe_path = QApplication::instance()->applicationDirPath().toStdString();
        return Utils::paramsToVector(exe_path + "/" + folder, paramsName);
    }

    static void ccdToDqeNtf(std::string fileName, std::string& name, std::vector<float>& dqe_io, std::vector<float>& ntf_io, std::string folder = "ccds")
    {
        std::string exe_path = QApplication::instance()->applicationDirPath().toStdString();
        std::ifstream inStream(exe_path + "/" + folder + "/" + fileName);
        Utils::ccdToDqeNtf(exe_path + "/" + folder, fileName, name, dqe_io, ntf_io);
    }

    template <typename T>
    static QString numToQString(T num, int prec = 10, bool isFixed = false) {
        return QString::fromStdString(Utils::numToString(num, prec, isFixed));
    }
};


#endif //CLTEM_GUI_STRINGUTILS_H
