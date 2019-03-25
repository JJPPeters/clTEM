#include <utility>

//
// Created by jon on 23/11/17.
//

#ifndef CLTEM_SIMULATIONWORKER_H
#define CLTEM_SIMULATIONWORKER_H

#include <complex>

#include "threadworker.h"
#include "clwrapper.h"
#include "utilities/logging.h"

template <class T>
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

    void initialiseProbeWave(double posx, double posy, int n_parallel = 0);

    void doMultiSliceStep(int slice);

    void simulateCtemImage();

    void simulateCtemImage(std::vector<T> dqe_data, std::vector<T> ntf_data, int binning, double doseperpix, double conversionfactor = 1);

    std::vector<double> getDiffractionImage(int parallel_ind = 0);

    std::vector<double> getExitWaveImage(unsigned int t = 0, unsigned int l = 0, unsigned int b = 0, unsigned int r = 0);

    std::vector<double> getCtemImage();

    double doSumReduction(clMemory<T, Manual> data, clWorkGroup globalSizeSum,
                         clWorkGroup localSizeSum, unsigned int nGroups, int totalSize);

    double getStemPixel(double inner, double outer, double xc, double yc, int parallel_ind);

    // OpenCL stuff
    clMemory<T, Manual> ClParameterisation;

    clMemory<T, Manual> ClAtomX;
    clMemory<T, Manual> ClAtomY;
    clMemory<T, Manual> ClAtomZ;
    clMemory<int, Manual> ClAtomA;

    clMemory<int, Manual> ClBlockStartPositions;
    clMemory<int, Manual> ClBlockIds;
    clMemory<int, Manual> ClZIds;

    std::vector<clMemory<std::complex<T>, Manual>> clWaveFunction1;
    std::vector<clMemory<std::complex<T>, Manual>> clWaveFunction2;
    clMemory<std::complex<T>, Manual> clWaveFunction3;
    std::vector<clMemory<std::complex<T>, Manual>> clWaveFunction4;

    clMemory<T, Manual> clXFrequencies;
    clMemory<T, Manual> clYFrequencies;
    clMemory<std::complex<T>, Manual> clPropagator;
    clMemory<std::complex<T>, Manual> clPotential;
    clMemory<std::complex<T>, Manual> clImageWaveFunction;

    // General kernels
    clFourier<T> FourierTrans;
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
    clMemory<T, Manual> clCcdBuffer;
    clMemory<std::complex<T>, Manual> clTempBuffer;

    // CBED
    clKernel InitProbeWavefunction;
    clMemory<T, Manual> clTDSMaskDiff;

    // STEM
    clKernel TDSMaskingAbsKernel;
    clKernel SumReduction;
    clMemory<T, Manual> clReductionBuffer;
};


#endif //CLTEM_SIMULATIONWORKER_H
