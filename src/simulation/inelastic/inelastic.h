//
// Created by jonat on 15/05/2020.
//

#ifndef CLTEM_INELASTIC_H
#define CLTEM_INELASTIC_H

#include <memory>
#include <inelastic/plasmon.h>
#include <inelastic/phonon.h>

class InelasticScattering {
private:
    unsigned int inelastic_iterations;

    std::shared_ptr<PlasmonScattering> plasmons;
    std::shared_ptr<PhononScattering> phonons;

public:

    InelasticScattering() {
        inelastic_iterations = 1;
        plasmons = std::make_shared<PlasmonScattering>();
        phonons = std::make_shared<PhononScattering>();
    }

    std::shared_ptr<PlasmonScattering> getPlasmons() {return plasmons;}
    std::shared_ptr<PhononScattering> getPhonons() {return phonons;}

//    void setPhononsEnabled(bool enabled) {phonons->frozen_phonon_enabled = enabled;}
    void setInelasticIterations(unsigned int it) {inelastic_iterations = it;}

//    bool getPhononEnabled() { return phonons->getFrozenPhononEnabled(); }
//    bool getPlasmonEnabled() { return plasmons->getPlasmonEnabled(); }
    bool getInelasticEnabled(){ return phonons->getFrozenPhononEnabled() || plasmons->getPlasmonEnabled(); }

    unsigned int getStoredInelasticInterations() { return inelastic_iterations;}
    unsigned int getInelasticIterations() {
        if (getInelasticEnabled())
            return inelastic_iterations;
        else
            return 1;
    }

};


#endif //CLTEM_INELASTIC_H
