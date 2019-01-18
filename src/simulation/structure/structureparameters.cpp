#include "structureparameters.h"

std::vector<std::string> StructureParameters::Names;

std::vector<std::vector<float>> StructureParameters::Params;

std::mutex StructureParameters::mtx;