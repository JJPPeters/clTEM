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
    std::string kernelToChar(std::string kernelName);

    void readParamsFile(const std::string &paramsName, std::string folder = "params");

    void ccdToDqeNtf(std::string fileName, std::string& name, std::vector<double>& dqe_io, std::vector<double>& ntf_io, std::string folder = "ccds");

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
