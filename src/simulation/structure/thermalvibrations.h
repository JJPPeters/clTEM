//
// Created by Jon on 29/05/2018.
//

#ifndef CLTEM_THERMAL_H
#define CLTEM_THERMAL_H

#include <vector>
#include <stdexcept>

#include "utilities/structureutils.h"

class ThermalVibrations {

    // this default is not used for the calculations, but is just a record of what we have set it as
    // the defaults will be filled into the vector of displacements
    static float u_default;

    // this is where all the parameters are set
    static std::vector<float> u_squareds;

    static std::vector<int> set_elements;

public:

    // if this is set, then the default value is used for everything
    static bool force_default;

    // if this is set, then the values defined here override values set by the file
    static bool force_defined;

    static std::vector<float> getDefinedVibrations() {
        std::vector<float> vals;
        for (const auto& i : set_elements) {
            vals.emplace_back(getVibrations(i)); // -1 as hydrogen is 1, but element 0, etc...
        }

        return vals;
    }

    static std::vector<int> getDefinedElements() {
        return set_elements;
    }

    static void setVibrations(float def, std::vector<int> elements, std::vector<float> vibs) {
        if (elements.size() != vibs.size())
            throw(std::runtime_error("Cannot set thermal vibrations with different elements and vibration vector sizes"));

        u_default = def;
        // reset to our default
        if (u_squareds.empty())
            u_squareds = std::vector<float>(Utils::VectorSymbolToNumber.size(), u_default);
        std::fill(u_squareds.begin(), u_squareds.end(), u_default);

        // fill in the non default values
        for(int i = 0; i < elements.size(); ++i) {
            if (elements[i] >= u_squareds.size())
                throw(std::runtime_error("Cannot set thermal parameters for element: " + std::to_string(elements[i])));
            u_squareds[elements[i]-1] = vibs[i]; // -1 as Hydrogen is 1 (not 0)
        }

        set_elements = elements;
    }

    static float getVibrations(unsigned int element) {
        if (u_squareds.empty())
            u_squareds = std::vector<float>(Utils::VectorSymbolToNumber.size(), u_default);

        if (element >= u_squareds.size())
            throw(std::runtime_error("Cannot access thermal parameters for element: " + std::to_string(element)));

        return u_squareds[element-1]; // -1 as hydrogen is 1, but element 0
    }

    static float getDefault(){
        return u_default;
    }
};


#endif //CLTEM_THERMAL_H
