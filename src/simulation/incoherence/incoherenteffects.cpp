//
// Created by jonat on 15/05/2020.
//

#include "incoherenteffects.h"

IncoherentEffects::IncoherentEffects() {
    incoherent_iterations = 1;
    plasmon_scattering = std::make_shared<PlasmonScattering>();
    phonon_scattering = std::make_shared<PhononScattering>();
    chromatic_effects = std::make_shared<ChromaticAberration>();
    source_size = std::make_shared<ProbeSourceSize>();
}

IncoherentEffects::IncoherentEffects(const IncoherentEffects &sm) {
    incoherent_iterations = sm.incoherent_iterations;
    plasmon_scattering = std::make_shared<PlasmonScattering>(*(sm.plasmon_scattering));
    phonon_scattering = std::make_shared<PhononScattering>(*(sm.phonon_scattering));
    chromatic_effects = std::make_shared<ChromaticAberration>(*(sm.chromatic_effects));
    source_size = std::make_shared<ProbeSourceSize>(*(sm.source_size));
}

IncoherentEffects &IncoherentEffects::operator=(const IncoherentEffects &sm) {
    incoherent_iterations = sm.incoherent_iterations;
    plasmon_scattering = std::make_shared<PlasmonScattering>(*(sm.plasmon_scattering));
    phonon_scattering = std::make_shared<PhononScattering>(*(sm.phonon_scattering));
    chromatic_effects = std::make_shared<ChromaticAberration>(*(sm.chromatic_effects));
    source_size = std::make_shared<ProbeSourceSize>(*(sm.source_size));

    return *this;
}
