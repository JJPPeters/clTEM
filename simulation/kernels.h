//
// Created by jon on 24/11/17.
//

#ifndef CLTEM_KERNELS_H
#define CLTEM_KERNELS_H


#include <string>

/// A simple class to hold the kernels that should be loaded at the start of program execution
// TODO: perhaps add a test to see if the kernels actually contain something?
struct Kernels
{

//    static char *atom_sort;
//    static char *floatSumReductionsource2;
//    static char *BandLimitSource;
//    static char *fftShiftSource;
//    static char *opt2source;
//    static char *fd2source;
//    static char *conv2source;
//    static char *propsource;
//    static char *multisource;
//    static char *gradsource;
//    static char *fdsource;
//    static char *InitialiseWavefunctionSource;
//    static char *imagingKernelSource;
//    static char *InitialiseSTEMWavefunctionSourceTest;
//    static char *floatabsbandPassSource;
//    static char *SqAbsSource;

    static std::string atom_sort;
    static std::string floatSumReductionsource2;
    static std::string BandLimitSource;
    static std::string fftShiftSource;
    static std::string opt2source;
    static std::string fd2source;
    static std::string conv2source;
    static std::string propsource;
    static std::string multisource;
    static std::string gradsource;
    static std::string fdsource;
    static std::string InitialiseWavefunctionSource;
    static std::string imagingKernelSource;
    static std::string InitialiseSTEMWavefunctionSourceTest;
    static std::string floatabsbandPassSource;
    static std::string SqAbsSource;

};

#endif //CLTEM_KERNELS_H
