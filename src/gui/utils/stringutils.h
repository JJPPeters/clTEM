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

        if (inStream.fail())
            throw std::runtime_error("Error opening resource file: " + paramsName);

        std::vector<float> out;
        float p;

        while (inStream >> p)
            out.push_back(p);

        inStream.close();

        return out;
    }

    static void ccdToDqeNtf(std::string fileName, std::string& name, std::vector<float>& dqe_io, std::vector<float>& ntf_io, std::string folder = "ccds")
    {
        std::string exe_path = QApplication::instance()->applicationDirPath().toStdString();
        std::ifstream inStream(exe_path + "/" + folder + "/" + fileName);

        if (inStream.fail())
            throw std::runtime_error("Error opening resource file: " + fileName);

        std::string header_temp;
        std::string data_temp;
        float tmp;

        dqe_io = std::vector<float>();
        ntf_io = std::vector<float>();

        bool found_dqe = false;
        bool found_ntf = false;

        // first line is always the name;
        std::getline(inStream, name);

        while(std::getline(inStream, header_temp))
        {
            // clear whitespace, don't know how robust this is. See https://stackoverflow.com/questions/83439/remove-spaces-from-stdstring-in-c
            header_temp.erase(std::remove_if(header_temp.begin(), header_temp.end(), isspace), header_temp.end());

            if (header_temp == "DQE")
            {
                if (std::getline(inStream, data_temp))
                {
                    std::stringstream data_stream(data_temp);
                    while (data_stream >> tmp)
                        dqe_io.push_back(tmp);
                    if (dqe_io.size() == 725)
                        found_dqe = true;
                }
            }
            else if (header_temp == "NTF")
            {
                if (std::getline(inStream, data_temp))
                {
                    std::stringstream data_stream(data_temp);
                    while (data_stream >> tmp)
                        ntf_io.push_back(tmp);
                    if (ntf_io.size() == 725)
                        found_ntf = true;
                }
            }
        }

        if (!found_dqe || !found_ntf)
            throw std::runtime_error("Could not find DQE and NTF in file: " + fileName);
    }

    template <typename T>
    static QString numToQString(T num, int prec = 10, bool isFixed = false) {
        return QString::fromStdString(Utils::numToString(num, prec, isFixed));
    }
};


#endif //CLTEM_GUI_STRINGUTILS_H
