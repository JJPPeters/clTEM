//
// Created by jonat on 24/05/2020.
//

#ifndef CLTEM_PROBESOURCESIZE_H
#define CLTEM_PROBESOURCESIZE_H

#include <random>
#include <chrono>

class ProbeSourceSize {
private:
    std::mt19937_64 rng;
    std::normal_distribution<> dist;

    bool is_enabled;

    double standard_deviation;

public:
    ProbeSourceSize();

    bool enabled() {return is_enabled;}
    void setEnabled(bool e) {is_enabled = e;}

    void setStandardDeviation(double sigma);
    void setFullWidthHalfMax(double fwhm);

    double standardDeviation();
    double fullWidthHalfMax();

    double getOffset();
};


#endif //CLTEM_PROBESOURCESIZE_H
