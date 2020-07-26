//
// Created by Jon on 31/01/2020.
//

#ifndef CLTEM_SIMULATIONCBED_H
#define CLTEM_SIMULATIONCBED_H

#include "simulationctem.h"
#include "kernels.h"

template <class GPU_Type>
class SimulationCbed : public SimulationCtem<GPU_Type>
{
protected:
    using SimulationGeneral<GPU_Type>::pool;
    using SimulationGeneral<GPU_Type>::job;
    using SimulationGeneral<GPU_Type>::last_mode;
    using SimulationGeneral<GPU_Type>::ctx;

    using SimulationGeneral<GPU_Type>::clWaveFunctionReal;
    using SimulationGeneral<GPU_Type>::clWaveFunctionRecip;
    using SimulationGeneral<GPU_Type>::clXFrequencies;
    using SimulationGeneral<GPU_Type>::clYFrequencies;
    using SimulationGeneral<GPU_Type>::FourierTrans;

    using SimulationGeneral<GPU_Type>::doMultiSliceStep;
    using SimulationGeneral<GPU_Type>::modifyBeamTilt;
    using SimulationGeneral<GPU_Type>::getDiffractionImage;

    using SimulationGeneral<GPU_Type>::reference_perturb_x;
    using SimulationGeneral<GPU_Type>::reference_perturb_y;

    void initialiseProbeWave(double posx, double posy, int n_parallel = 0);

    bool initialiseSimulation();

    void initialiseBuffers();
    void initialiseKernels();

    bool do_initialise_cbed;

public:
    explicit SimulationCbed(const clContext &_ctx, ThreadPool &s, unsigned int _id) : SimulationCtem<GPU_Type>(_ctx, s, _id), do_initialise_cbed(true) {}

    ~SimulationCbed() = default;

    void simulate();

private:

    clKernel InitProbeWavefunction;
};


#endif //CLTEM_SIMULATIONCBED_H
