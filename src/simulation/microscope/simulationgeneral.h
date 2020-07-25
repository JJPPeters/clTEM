//
// Created by Jon on 31/01/2020.
//

#ifndef CLTEM_SIMULATIONGENERAL_H
#define CLTEM_SIMULATIONGENERAL_H

#include "clwrapper.h"

#include "kernels.h"
#include <threading/simulationjob.h>
#include "simulationmanager.h"
#include "threading/threadworker.h"
#include <utilities/simutils.h>

template <class GPU_Type>
class SimulationGeneral : public ThreadWorker
{
public:
    explicit SimulationGeneral(const clContext &_ctx, ThreadPool &s, unsigned int _id)
        : ThreadWorker(s, _id), ctx(_ctx),
        last_mode(SimulationMode::None), last_do_3d(false), do_initialise_general(true),
          reference_perturb_x(0.0), reference_perturb_y(0.0) {

    }

    ~SimulationGeneral() {ctx.WaitForQueueFinish(); ctx.WaitForIOQueueFinish();}

protected:
    SimulationMode last_mode;
    bool last_do_3d;
    bool last_double_precision;
    bool do_initialise_general;

    // these are used to perturb the reference frame (i.e. when moving the source)
    double reference_perturb_x, reference_perturb_y;

    clContext ctx;

    // this is only used to check if the manager has changed (only to avoid sorting atoms multiple times)
    // TODO: could be more specific, instead of testing the whole manager?
    std::shared_ptr<SimulationManager> current_manager;

    std::shared_ptr<SimulationJob> job;

    void sortAtoms();

    void initialiseSimulation();

    void doMultiSliceStep(int slice);

    std::vector<double> getDiffractionImage(int parallel_ind, double d_kx = 0.0, double d_ky = 0.0);

    std::vector<double> getExitWaveImage(unsigned int t = 0, unsigned int l = 0, unsigned int b = 0, unsigned int r = 0);

    virtual void simulate() = 0;

    void initialiseBuffers();
    void initialiseKernels();

    // this tilts the beam mid simulation - used for plasmon scattering.
    void modifyBeamTilt(double kx, double ky, double kz);

    void translateDiffImage(double d_kx, double d_ky);

    // OpenCL stuff
    clMemory<GPU_Type, Manual> ClParameterisation;

    clMemory<GPU_Type, Manual> ClAtomX;
    clMemory<GPU_Type, Manual> ClAtomY;
    clMemory<GPU_Type, Manual> ClAtomZ;
    clMemory<int, Manual> ClAtomA;

    clMemory<int, Manual> ClBlockStartPositions;
    clMemory<int, Manual> ClBlockIds;
    clMemory<int, Manual> ClZIds;

    std::vector<clMemory<std::complex<GPU_Type>, Manual>> clWaveFunctionReal;
    std::vector<clMemory<std::complex<GPU_Type>, Manual>> clWaveFunctionRecip;
    clMemory<std::complex<GPU_Type>, Manual> clWaveFunctionTemp_1;
    clMemory<GPU_Type, Manual> clWaveFunctionTemp_2;
    clMemory<GPU_Type, Manual> clWaveFunctionTemp_3;

    clMemory<GPU_Type, Manual> clXFrequencies;
    clMemory<GPU_Type, Manual> clYFrequencies;
    clMemory<std::complex<GPU_Type>, Manual> clPropagator;
//    clMemory<std::complex<GPU_Type>, Manual> clTransmissionFunction;
    std::vector<clMemory<std::complex<GPU_Type>, Manual>> clTransmissionFunction;

    // General kernels
    clFourier<GPU_Type> FourierTrans;
    clKernel AtomSort;
    clKernel BandLimit;
    clKernel FftShift;
    clKernel CalculateTransmissionFunction;
    clKernel GeneratePropagator;
    clKernel ComplexMultiply;
    clKernel BilinearTranslate;
    clKernel ComplexToReal;
};


#endif //CLTEM_SIMULATIONGENERAL_H
