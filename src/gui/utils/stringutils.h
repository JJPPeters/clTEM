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
#include "guiutils.h"

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
    static QString numToQString(T num, int prec = GuiUtils::edit_precision, bool isFixed = true) {
        // https://stackoverflow.com/questions/15165502/double-to-string-without-scientific-notation-or-trailing-zeros-efficiently
        // use fixed, but remove trailing zeros/decimals
        std::string s = Utils::numToString(num, prec, isFixed);

        if (s.find(".") != std::string::npos) { // only remove trailing decimals
            s.erase(s.find_last_not_of('0') + 1, std::string::npos); //remove trailing 000s
            if (s[s.size() - 1] == '.')
                s = s.substr(0, s.size() - 1); //remove dangling decimal
        }
        return QString::fromStdString(s);
    }
};


#endif //CLTEM_GUI_STRINGUTILS_H
