//
// Created by Jon on 29/05/2018.
//

#include "phonon.h"

PhononScattering::PhononScattering() {
    dist = std::normal_distribution<>(0, 1);
    rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());

    frozen_phonon_enabled = false;
    force_default = false;
    force_defined = false;
    u_default = 0.0f;
    set_elements = {};
    u_squareds = std::vector<double>(Utils::VectorSymbolToNumber.size(), u_default);
}

PhononScattering::PhononScattering(const PhononScattering &ps) {
    dist = std::normal_distribution<>(0, 1);
    rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());

    frozen_phonon_enabled = ps.frozen_phonon_enabled;
    force_default = ps.force_default;
    force_defined = ps.force_defined;
    u_default = ps.u_default;
    set_elements = ps.set_elements;
    u_squareds = ps.u_squareds;
}

PhononScattering& PhononScattering::operator=(const PhononScattering &ps) {
    dist = std::normal_distribution<>(0, 1);
    rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());

    frozen_phonon_enabled = ps.frozen_phonon_enabled;
    force_default = ps.force_default;
    force_defined = ps.force_defined;
    u_default = ps.u_default;
    set_elements = ps.set_elements;
    u_squareds = ps.u_squareds;

    return *this;
}

std::vector<double> PhononScattering::getDefinedVibrations() {
    std::vector<double> vals;
    for (const auto& i : set_elements) {
        vals.emplace_back(getVibrations(i)); // -1 as hydrogen is 1, but element 0, etc...
    }

    return vals;
}

void PhononScattering::setVibrations(double def, std::vector<int> elements, std::vector<double> vibs) {
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

void PhononScattering::setDefault(double def) {
    // here we need to account for the way we store the vibrations for EVERY element
    std::vector<double> vibrations(set_elements.size());

    for(int i = 0; i < set_elements.size(); ++i) {
        vibrations[i] = u_squareds[set_elements[i]-1]; // -1 as hydrogen is 1, but element 0
    }

    setVibrations(def, set_elements, vibrations);
}

double PhononScattering::getVibrations(unsigned int element) {
    if (u_squareds.empty())
        u_squareds = std::vector<double>(Utils::VectorSymbolToNumber.size(), u_default);

    if (element >= u_squareds.size())
        throw(std::runtime_error("Cannot access thermal parameters for element: " + std::to_string(element)));

    return u_squareds[element-1]; // -1 as hydrogen is 1, but element 0
}

double PhononScattering::generateTdsFactor(AtomSite& at, int direction) {
    if (direction < 0 || direction > 2)
        throw std::runtime_error("Error trying to apply thermal displacement to axis: " + std::to_string(direction));

    // need element (just pass atom?)

    // TODO: check this behaves as expected, may want to reset the random stuff
    // sqrt as we have the mean squared displacement (variance), but want the standard deviation

    double u = 0.0;

    if ( force_default )
        u = getDefault();
    else if (force_defined)
        u = getVibrations((unsigned int) at.A);
    else if (at.defined_u) {
        if (direction == 0)
            u = at.ux;
        else if (direction == 1)
            u = at.uy;
        else if (direction == 2)
            u = at.uz;
    } else {
        // defaults are built into this
        u = getVibrations((unsigned int) at.A);
    }

    auto jj = dist(rng);
    double randNormal = std::sqrt(u) * jj;

    return randNormal;
}
