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

    // if this is set, then the default value is used for everything
    bool force_default;

    // if this is set, then the values defined here override values set by the file
    bool force_defined;

    //
    bool force_xyz_thermal_disps;

public:

    PhononScattering();
    PhononScattering(const PhononScattering& ps);
    PhononScattering& operator=(const PhononScattering& ps);

    void setFrozenPhononEnabled(bool enabled) {frozen_phonon_enabled = enabled;}
    bool getFrozenPhononEnabled() {return frozen_phonon_enabled;}

    double generateTdsFactor(AtomSite& at, int direction);

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

    bool forceDefined() {return force_defined;}

    void setForceDefined(bool set) {
        force_defined = set;
    }

    bool forceDefault() {return force_default;}

    void setForceDefault(bool set) {
        force_default = set;
    }

    bool forceXyzDisps() {return force_xyz_thermal_disps;}

    void setForceXyzDisps(bool set) {
        force_xyz_thermal_disps = set;
    }
};


#endif //CLTEM_THERMAL_H
