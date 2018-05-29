//
// Created by Jon on 29/05/2018.
//

#include "thermalvibrations.h"

bool ThermalVibrations::force_default = false;

bool ThermalVibrations::force_defined = false;

float ThermalVibrations::u_default = 0.0f;

std::vector<float> ThermalVibrations::u_squareds = std::vector<float>(Utils::MapSymbolToNumber.size(), ThermalVibrations::u_default);