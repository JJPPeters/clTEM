//
// Created by Jon on 29/05/2018.
//

#include "thermalvibrations.h"

ThermalVibrations::ThermalVibrations() {
    force_default = false;
    force_defined = false;
    u_default = 0.0f;
    std::vector<int> set_elements = {};
    std::vector<double> u_squareds = std::vector<double>(Utils::VectorSymbolToNumber.size(), u_default);
}

std::vector<double> ThermalVibrations::getDefinedVibrations() {
    std::vector<double> vals;
    for (const auto& i : set_elements) {
        vals.emplace_back(getVibrations(i)); // -1 as hydrogen is 1, but element 0, etc...
    }

    return vals;
}

void ThermalVibrations::setVibrations(double def, std::vector<int> elements, std::vector<double> vibs) {
    if (elements.size() != vibs.size())
        throw(std::runtime_error("Cannot set thermal vibrations with different elements and vibration vector sizes"));

    u_default = def;
    // reset to our default, check we have the vector is need be
    if (u_squareds.empty())
        u_squareds = std::vector<double>(Utils::VectorSymbolToNumber.size(), u_default);
    std::fill(u_squareds.begin(), u_squareds.end(), u_default);

    // fill in the non default values
    for(int i = 0; i < elements.size(); ++i) {
        if (elements[i] >= u_squareds.size())
            throw(std::runtime_error("Cannot set thermal parameters for element: " + std::to_string(elements[i])));
        u_squareds[elements[i]-1] = vibs[i]; // -1 as Hydrogen is 1 (not 0)
    }

    set_elements = elements;
}

double ThermalVibrations::getVibrations(unsigned int element) {
    if (u_squareds.empty())
        u_squareds = std::vector<double>(Utils::VectorSymbolToNumber.size(), u_default);

    if (element >= u_squareds.size())
        throw(std::runtime_error("Cannot access thermal parameters for element: " + std::to_string(element)));

    return u_squareds[element-1]; // -1 as hydrogen is 1, but element 0
}