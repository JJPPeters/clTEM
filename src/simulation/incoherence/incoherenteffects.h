//
// Created by jonat on 15/05/2020.
//

#ifndef CLTEM_INCOHERENTEFFECTS_H
#define CLTEM_INCOHERENTEFFECTS_H

#include <memory>

#include "inelastic/plasmon.h"
#include "inelastic/phonon.h"

#include "chromaticaberration.h"
#include "probesourcesize.h"

class IncoherentEffects {
private:
    unsigned int incoherent_iterations;

    std::shared_ptr<PlasmonScattering> plasmon_scattering;
    std::shared_ptr<PhononScattering> phonon_scattering;

    std::shared_ptr<ChromaticAberration> chromatic_effects;
    std::shared_ptr<ProbeSourceSize> source_size;

public:

    IncoherentEffects() {
        incoherent_iterations = 1;
        plasmon_scattering = std::make_shared<PlasmonScattering>();
        phonon_scattering = std::make_shared<PhononScattering>();
        chromatic_effects = std::make_shared<ChromaticAberration>();
        source_size = std::make_shared<ProbeSourceSize>();
    }

    std::shared_ptr<PlasmonScattering> plasmons() {return plasmon_scattering;}
    std::shared_ptr<PhononScattering> phonons() {return phonon_scattering;}
    std::shared_ptr<ChromaticAberration> chromatic() {return chromatic_effects;}
    std::shared_ptr<ProbeSourceSize> source() {return source_size;}

    bool enabled() {
        return phonon_scattering->getFrozenPhononEnabled() || plasmon_scattering->enabled() || chromatic_effects->enabled() || source_size->enabled();
    }

    unsigned int iterations() {
        if (enabled())
            return incoherent_iterations;
        else
            return 1;
    }

    [[nodiscard]] unsigned int storedIterations() const {
        return incoherent_iterations;
    }

    void setIterations(unsigned int it) {
        incoherent_iterations = it;
    }

};


#endif //CLTEM_INCOHERENTEFFECTS_H
