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
    float u_default;

    // this is where all the parameters are set
    std::vector<float> u_squareds;

    std::vector<int> set_elements;

public:

    ThermalVibrations();

    // if this is set, then the default value is used for everything
    bool force_default;

    // if this is set, then the values defined here override values set by the file
    bool force_defined;

    std::vector<float> getDefinedVibrations();

    std::vector<int> getDefinedElements() {
        return set_elements;
    }

    void setVibrations(float def, std::vector<int> elements, std::vector<float> vibs);

    float getVibrations(unsigned int element);

    float getDefault(){
        return u_default;
    }
};


#endif //CLTEM_THERMAL_H
