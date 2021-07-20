//
// Created by Jon on 31/01/2020.
//

#ifndef CLTEM_SIMULATIONCTEM_H
#define CLTEM_SIMULATIONCTEM_H

#include "simulationgeneral.h"
#include "ccdparams.h"

template <class GPU_Type>
class SimulationCtem : public SimulationGeneral<GPU_Type>
{
protected:
    using SimulationGeneral<GPU_Type>::pool;
    using SimulationGeneral<GPU_Type>::job;
    using SimulationGeneral<GPU_Type>::last_mode;
    using SimulationGeneral<GPU_Type>::ctx;

    using SimulationGeneral<GPU_Type>::clWaveFunctionReal;
    using SimulationGeneral<GPU_Type>::clWaveFunctionRecip;
    using SimulationGeneral<GPU_Type>::clWaveFunctionTemp_1;
    using SimulationGeneral<GPU_Type>::clXFrequencies;
    using SimulationGeneral<GPU_Type>::clYFrequencies;
    using SimulationGeneral<GPU_Type>::FourierTrans;

    using SimulationGeneral<GPU_Type>::doMultiSliceStep;
    using SimulationGeneral<GPU_Type>::modifyBeamTilt;
    using SimulationGeneral<GPU_Type>::getDiffractionImage;
    using SimulationGeneral<GPU_Type>::getExitWaveImage;

    using SimulationGeneral<GPU_Type>::reference_perturb_x;
    using SimulationGeneral<GPU_Type>::reference_perturb_y;

    void initialiseBuffers();
    void initialiseKernels();

    bool do_initialise_ctem;

public:
    explicit SimulationCtem(clDevice &_dev, ThreadPool &s, unsigned int _id) : SimulationGeneral<GPU_Type>(_dev, s, _id), do_initialise_ctem(true) {}

    ~SimulationCtem() {ctx->WaitForQueueFinish(); ctx->WaitForIOQueueFinish();}

    void simulate();

private:
    bool initialiseSimulation();

    void simulateCtemImage();

    void simulateImagePerfect();

    void simulateImageDose(std::vector<GPU_Type> dqe_data, std::vector<GPU_Type> ntf_data, int binning, double doseperpix, double conversionfactor = 1);

    std::vector<double> getCtemImage();

    clMemory<std::complex<GPU_Type>, Manual> clImageWaveFunction;

    clKernel InitPlaneWavefunction;
    clKernel ImagingKernel;
    clKernel ABS2;
    clKernel NtfKernel;
    clKernel DqeKernel;
    clMemory<GPU_Type, Manual> clCcdBuffer;
    clMemory<std::complex<GPU_Type>, Manual> clTempBuffer;
};


#endif //CLTEM_SIMULATIONCTEM_H
