#include <utility>

//
// Created by jon on 23/11/17.
//

#ifndef CLTEM_SIMULATIONWORKER_H
#define CLTEM_SIMULATIONWORKER_H


#include "threadworker.h"
#include "clwrapper.h"
#include "utilities/logging.h"


class SimulationWorker : public ThreadWorker
{
private:

    SimulationMode last_mode;
    bool last_do_3d;
    bool do_initialise;

    void initialiseBuffers();
    void initialiseKernels();

public:
    // initialise FourierTrans just with any values
    SimulationWorker(ThreadPool &s, unsigned int _id, const clContext &_ctx) : ThreadWorker(s, _id), ctx(_ctx), last_mode(SimulationMode::None), last_do_3d(false), do_initialise(true) {}

    ~SimulationWorker() {ctx.WaitForQueueFinish(); ctx.WaitForIOQueueFinish();}

    void Run(const std::shared_ptr<SimulationJob> &_job) override;

private:
    clContext ctx;

    std::shared_ptr<SimulationJob> job;

    void sortAtoms(bool doTds = false);

    void doCtem(bool simImage = false);

    void doCbed();

    void doStem();

    void initialiseSimulation();

    void initialiseCtem();

    void initialiseProbeWave(float posx, float posy, int n_parallel = 0);

    void doMultiSliceStep(int slice);

    void simulateCtemImage();

    void simulateCtemImage(std::vector<float> dqe_data, std::vector<float> ntf_data, int binning, float doseperpix,
                           float conversionfactor = 1);

    std::vector<float> getDiffractionImage(int parallel_ind = 0);

    std::vector<float> getExitWaveImage(unsigned int t = 0, unsigned int l = 0, unsigned int b = 0, unsigned int r = 0);

    std::vector<float> getCtemImage();

    float doSumReduction(clMemory<float, Manual> data, clWorkGroup globalSizeSum,
                         clWorkGroup localSizeSum, unsigned int nGroups, int totalSize);

    float getStemPixel(float inner, float outer, float xc, float yc, int parallel_ind);

    // OpenCL stuff
    clMemory<float, Manual> ClParameterisation;

    clMemory<float, Manual> ClAtomX;
    clMemory<float, Manual> ClAtomY;
    clMemory<float, Manual> ClAtomZ;
    clMemory<int, Manual> ClAtomA;

    clMemory<int, Manual> ClBlockStartPositions;
    clMemory<int, Manual> ClBlockIds;
    clMemory<int, Manual> ClZIds;

    std::vector<clMemory<cl_float2, Manual>> clWaveFunction1;
    std::vector<clMemory<cl_float2, Manual>> clWaveFunction2;
    clMemory<cl_float2, Manual> clWaveFunction3;
    std::vector<clMemory<cl_float2, Manual>> clWaveFunction4;

    clMemory<float, Manual> clXFrequencies;
    clMemory<float, Manual> clYFrequencies;
    clMemory<cl_float2, Manual> clPropagator;
    clMemory<cl_float2, Manual> clPotential;
    clMemory<cl_float2, Manual> clImageWaveFunction;

    // General kernels
    clFourier FourierTrans;
    clKernel AtomSort;
    clKernel BandLimit;
    clKernel fftShift;
    clKernel BinnedAtomicPotential;
    clKernel GeneratePropagator;
    clKernel ComplexMultiply;

    // CTEM
    clKernel InitPlaneWavefunction;
    clKernel ImagingKernel;
    clKernel ABS2;
    clKernel NtfKernel;
    clKernel DqeKernel;
    clMemory<float, Manual> clCcdBuffer;
    clMemory<cl_float2, Manual> clTempBuffer;

    // CBED
    clKernel InitProbeWavefunction;
    clMemory<float, Manual> clTDSMaskDiff;

    // STEM
    clKernel TDSMaskingAbsKernel;
    clKernel SumReduction;
    clMemory<float, Manual> clReductionBuffer;
};


#endif //CLTEM_SIMULATIONWORKER_H
