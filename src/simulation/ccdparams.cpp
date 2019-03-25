//
// Created by jon on 02/04/18.
//

#include "ccdparams.h"

std::vector<std::vector<double>> CCDParams::dqes;

std::vector<std::vector<double>> CCDParams::ntfs;

std::vector<std::string> CCDParams::names;

std::mutex CCDParams::mtx;