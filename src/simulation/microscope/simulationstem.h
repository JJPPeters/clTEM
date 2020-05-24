//
// Created by Jon on 31/01/2020.
//

#ifndef CLTEM_SIMULATIONSTEM_H
#define CLTEM_SIMULATIONSTEM_H

#include "simulationcbed.h"

template <class GPU_Type>
class SimulationStem : public SimulationCbed<GPU_Type>
{
protected:
    using SimulationGeneral<GPU_Type>::pool;
    using SimulationGeneral<GPU_Type>::job;
    using SimulationGeneral<GPU_Type>::last_mode;
    using SimulationGeneral<GPU_Type>::ctx;

    using SimulationGeneral<GPU_Type>::clWaveFunctionRecip;
    using SimulationGeneral<GPU_Type>::clWaveFunctionTemp_1;
    using SimulationGeneral<GPU_Type>::FftShift;

    using SimulationGeneral<GPU_Type>::doMultiSliceStep;
    using SimulationGeneral<GPU_Type>::modifyBeamTilt;
    using SimulationGeneral<GPU_Type>::ComplexToReal;

    using SimulationCbed<GPU_Type>::initialiseProbeWave;
    using SimulationCbed<GPU_Type>::clWaveFunctionTemp_2;
    using SimulationCbed<GPU_Type>::clWaveFunctionTemp_3;

    using SimulationGeneral<GPU_Type>::translateDiffImage;

    void initialiseSimulation();

    void initialiseBuffers();
    void initialiseKernels();

    clKernel BandPassAbs;
    clKernel SumReduction;
    clMemory<GPU_Type, Manual> clReductionBuffer;

    bool do_initialise_stem;

public:
    explicit SimulationStem(const clContext &_ctx, ThreadPool &s, unsigned int _id) : SimulationCbed<GPU_Type>(_ctx, s, _id), do_initialise_stem(true) {}

    ~SimulationStem() = default;

    void simulate();

private:
    double doSumReduction(clMemory<GPU_Type, Manual> data, clWorkGroup globalSizeSum,
                          clWorkGroup localSizeSum, unsigned int nGroups, int totalSize);

    double getStemPixel(double inner, double outer, double xc, double yc, int parallel_ind, double d_kx=0.0, double d_ky=0.0);
};

#endif //CLTEM_SIMULATIONSTEM_H