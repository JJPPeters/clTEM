//
// Created by jonat on 15/05/2020.
//

#ifndef CLTEM_INCOHERENTEFFECTS_H
#define CLTEM_INCOHERENTEFFECTS_H

#include <memory>
#include <utilities/enums.h>

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

    IncoherentEffects();

    IncoherentEffects(const IncoherentEffects& sm);

    IncoherentEffects& operator=(const IncoherentEffects& sm);

    std::shared_ptr<PlasmonScattering> plasmons() {return plasmon_scattering;}
    std::shared_ptr<PhononScattering> phonons() {return phonon_scattering;}
    std::shared_ptr<ChromaticAberration> chromatic() {return chromatic_effects;}
    std::shared_ptr<ProbeSourceSize> source() {return source_size;}

    bool enabled(SimulationMode s_m) {
        bool enabled_all = phonon_scattering->getFrozenPhononEnabled() || plasmon_scattering->enabled();

        if (s_m != SimulationMode::CTEM)
            enabled_all = enabled_all || chromatic_effects->enabled() || source_size->enabled();

        return enabled_all;
    }

    unsigned int iterations(SimulationMode s_m) {
        if (enabled(s_m))
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
