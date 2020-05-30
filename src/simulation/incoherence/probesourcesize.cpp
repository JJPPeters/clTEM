//
// Created by jonat on 24/05/2020.
//

#include "probesourcesize.h"

ProbeSourceSize::ProbeSourceSize() {
    standard_deviation = 0.0;
    dist = std::normal_distribution<>(0, 1);
    rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());
}

void ProbeSourceSize::setStandardDeviation(double sigma) {
    standard_deviation = sigma;
}

void ProbeSourceSize::setFullWidthHalfMax(double fwhm) {
    standard_deviation = fwhm / (2.0 * std::sqrt( 2.0 * std::log(2.0)));
}

double ProbeSourceSize::standardDeviation() {
    return standard_deviation;
}

double ProbeSourceSize::fullWidthHalfMax() {
    return standard_deviation * 2.0 * std::sqrt( 2.0 * std::log(2.0));
}

double ProbeSourceSize::getOffset() {
    return dist(rng) * standard_deviation;
}
