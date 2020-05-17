//
// Created by Jon on 29/05/2018.
//

#ifndef CLTEM_THERMAL_H
#define CLTEM_THERMAL_H

#include <vector>
#include <stdexcept>
#include <structure/atom.h>
#include <random>
#include <chrono>

#include "utilities/structureutils.h"

class PhononScattering {
    bool frozen_phonon_enabled;

    // this default is not used for the calculations, but is just a record of what we have set it as
    // the defaults will be filled into the vector of displacements
    double u_default;

    // this is where all the parameters are set
    std::vector<double> u_squareds;

    std::vector<int> set_elements;

    std::mt19937_64 rng;
    std::normal_distribution<> dist;

public:

    PhononScattering();
    PhononScattering(const PhononScattering& ps);
    PhononScattering& operator=(const PhononScattering& ps);

    void setFrozenPhononEnabled(bool enabled) {frozen_phonon_enabled = enabled;}
    bool getFrozenPhononEnabled() {return frozen_phonon_enabled;}

    double generateTdsFactor(AtomSite& at, int direction);

    // if this is set, then the default value is used for everything
    bool force_default;

    // if this is set, then the values defined here override values set by the file
    bool force_defined;

    std::vector<double> getDefinedVibrations();

    std::vector<int> getDefinedElements() {
        return set_elements;
    }

    void setVibrations(double def, std::vector<int> elements, std::vector<double> vibs);

    void setDefault(double def);

    double getVibrations(unsigned int element);

    double getDefault(){
        return u_default;
    }
};


#endif //CLTEM_THERMAL_H
