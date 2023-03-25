//
// Created by jon on 09/12/17.
//

#include "stringutils.h"

std::string Utils_Qt::kernelToChar(std::string kernelName) {
    std::string full_path = QApplication::instance()->applicationDirPath().toStdString() + "/kernels";
    return Utils::resourceToChar(full_path, kernelName);
}

void Utils_Qt::readParamsFile(const std::string &paramsName, std::string folder) {
    std::string exe_path = QApplication::instance()->applicationDirPath().toStdString();
    Utils::readParams(exe_path + "/" + folder, paramsName);
}

void
Utils_Qt::ccdToDqeNtf(std::string fileName, std::string &name, std::vector<double> &dqe_io, std::vector<double> &ntf_io,
                      std::string folder) {
    std::string exe_path = QApplication::instance()->applicationDirPath().toStdString();
    std::ifstream inStream(exe_path + "/" + folder + "/" + fileName);
    Utils::ccdToDqeNtf(exe_path + "/" + folder, fileName, name, dqe_io, ntf_io);
}
