#include "structureparameters.h"

std::vector<std::string> StructureParameters::Names;

std::vector<std::vector<float>> StructureParameters::Params;

int StructureParameters::Current = -1;

std::mutex StructureParameters::mtx;