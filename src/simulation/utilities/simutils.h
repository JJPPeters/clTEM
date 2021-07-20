//
// Created by jonat on 27/01/2019.
//

#ifndef CLTEM_SIMUTILS_H
#define CLTEM_SIMUTILS_H

#include <memory>
#include <string>
#include <simulationmanager.h>
#include <utilities/fileio.h>

namespace Utils {
    enum ComplexDisplay {
        Real = 0,
        Imaginary = 1,
        Magnitude = 2,
        Angle = 3,
        AbsSquared = 4
    };

    bool checkSimulationPrerequisites(std::shared_ptr<SimulationManager> Manager, std::vector<clDevice> &Devices);

    // This is basically for debugging
    template<typename T>
    void saveComplexBuffer(std::string path, clMemory<std::complex<T>, Manual> buf, ComplexDisplay complex_type) {
        auto data = buf.GetLocal();
        std::vector<T> output(data.size());

        if (complex_type == ComplexDisplay::Real)
            for (int i = 0; i < data.size(); ++i)
                output[i] = std::real(data[i]);
        else if (complex_type == ComplexDisplay::Imaginary)
            for (int i = 0; i < data.size(); ++i)
                output[i] = std::imag(data[i]);
        else if (complex_type == ComplexDisplay::Magnitude)
            for (int i = 0; i < data.size(); ++i)
                output[i] = std::abs(data[i]);
        else if (complex_type == ComplexDisplay::Angle)
            for (int i = 0; i < data.size(); ++i)
                output[i] = std::arg(data[i]);
        else if (complex_type == ComplexDisplay::AbsSquared)
            for (int i = 0; i < data.size(); ++i)
                output[i] = std::abs(data[i]) * std::abs(data[i]);

        // default to square images (true for all buffers at the moment)
        unsigned int sz = std::sqrt(data.size());
        fileio::SaveTiff<float>(path, output, sz, sz);
    }

    template<typename T>
    void saveBuffer(std::string path, clMemory<T, Manual> buf) {
        auto output = buf.GetLocal();

        // default to square images (true for all buffers at the moment)
        unsigned int sz = std::sqrt(output.size());
        fileio::SaveTiff<float>(path, output, sz, sz);
    }
}

#endif //CLTEM_SIMUTILS_H
