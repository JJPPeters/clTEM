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

    std::shared_ptr<PlasmonScattering> plasmon_scattering;
    std::shared_ptr<PhononScattering> phonon_scattering;

public:

    InelasticScattering() {
        inelastic_iterations = 1;
        plasmon_scattering = std::make_shared<PlasmonScattering>();
        phonon_scattering = std::make_shared<PhononScattering>();
    }

    std::shared_ptr<PlasmonScattering> plasmons() {return plasmon_scattering;}
    std::shared_ptr<PhononScattering> phonons() {return phonon_scattering;}

    bool enabled() {
        return phonon_scattering->getFrozenPhononEnabled() || plasmon_scattering->enabled();
    }

    unsigned int iterations() {
        if (enabled())
            return inelastic_iterations;
        else
            return 1;
    }

    [[nodiscard]] unsigned int storedIterations() const {
        return inelastic_iterations;
    }

    void setIterations(unsigned int it) {
        inelastic_iterations = it;
    }

};


#endif //CLTEM_INELASTIC_H
