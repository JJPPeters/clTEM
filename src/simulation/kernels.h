#include <utility>

#include <utility>

#include <utility>

//
// Created by jon on 24/11/17.
//

#ifndef CLTEM_KERNELS_H
#define CLTEM_KERNELS_H


#include <string>
#include <mutex>
#include "clwrapper/clwrapper.h"
#include <regex>

/// A simple class to hold the kernels that should be loaded at the start of program execution
class KernelSource {
public:
    KernelSource() : mtx(), source(""), name(""), num_args(0) { }
    KernelSource(std::string _source, std::string _name, unsigned int _num_args) : mtx(), source(std::move(_source)), name(std::move(_name)), num_args(_num_args) { }
    explicit KernelSource(std::string _source) : mtx(), source(std::move(_source)), name(""), num_args(0) { parseKernelString(); }

    KernelSource& operator=(const KernelSource& rhs) {source = rhs.source; name = rhs.name; num_args = rhs.num_args;  return *this;}
    KernelSource& operator=(const std::string& rhs) { source = rhs; parseKernelString();
    return *this;}
    KernelSource(const KernelSource& ks) : mtx(), source(ks.source), name(ks.name), num_args(ks.num_args) { }

    std::string getSource() { std::lock_guard<std::mutex> lck(mtx); return source; }
    std::string getName() { std::lock_guard<std::mutex> lck(mtx); return name; }
    unsigned int getNumArgs() { std::lock_guard<std::mutex> lck(mtx); return num_args; }

    clKernel BuildToKernel(clContext ctx) {
        std::lock_guard<std::mutex> lck(mtx);
        return clKernel(ctx, source, name, num_args);
    }

private:
    std::mutex mtx;

    std::string source;
    std::string name;
    unsigned int num_args;

    void parseKernelString() {
        // This is designed to try and gather the information directly from the source using regex
        // This may not be perfect for all kernels, but works for the ones I have ever used
        std::regex name_regex(R"(__kernel(?:.|\s)+?\s+(.+)\s*\()"); //__kernel\s+?[a-zA-Z]+\s+?(.+?)\(
        std::regex args_regex(R"(__kernel(?:.|\s)+?\(((?:.|\s)*?)\))");
        std::smatch matches;

        if (std::regex_search(source, matches, name_regex)) {
            if (matches.size() != 2)
                throw std::runtime_error("Could not parse kernel to get kernel name");
            name = matches[1].str();
        }

        if (std::regex_search(source, matches, args_regex)) {
            if (matches.size() != 2)
                throw std::runtime_error("Could not parse kernel to get kernel arguments");
            std::string args_str = matches[1].str();
            num_args = static_cast<unsigned int>(std::count(args_str.begin(), args_str.end(), ',') + 1);
        }
    }
};

struct Kernels
{
    static KernelSource atom_sort_f;
    static KernelSource band_limit_f;
    static KernelSource band_pass_f;
    static KernelSource ccd_dqe_f;
    static KernelSource ccd_ntf_f;
    static KernelSource complex_multiply_f;
    static KernelSource ctem_image_f;
    static KernelSource fft_shift_f;
    static KernelSource init_plane_wave_f;
    static KernelSource init_probe_wave_f;
    static KernelSource transmission_potentials_full_3d_f;
    static KernelSource transmission_potentials_projected_f;
    static KernelSource propogator_f;
    static KernelSource sqabs_f;
    static KernelSource sum_reduction_f;

    static KernelSource atom_sort_d;
    static KernelSource band_limit_d;
    static KernelSource band_pass_d;
    static KernelSource ccd_dqe_d;
    static KernelSource ccd_ntf_d;
    static KernelSource complex_multiply_d;
    static KernelSource ctem_image_d;
    static KernelSource fft_shift_d;
    static KernelSource init_plane_wave_d;
    static KernelSource init_probe_wave_d;
    static KernelSource transmission_potentials_full_3d_d;
    static KernelSource transmission_potentials_projected_d;
    static KernelSource propogator_d;
    static KernelSource sqabs_d;
    static KernelSource sum_reduction_d;

};

#endif //CLTEM_KERNELS_H
