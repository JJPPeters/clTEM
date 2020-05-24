//
// Created by jonat on 24/05/2020.
//

#include <utilities/commonstructs.h>
#include "chromaticaberration.h"

ChromaticAberration::ChromaticAberration() {
    is_enabled = false;
    sigma_neg = 0.0;
    sigma_pos = 0.0;
    chromatic_aberration = 0.0;
    dist = std::normal_distribution<>(0, 1);
    dist_selector = std::uniform_real_distribution<>(0, 1);
    rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());
}

bool ChromaticAberration::enabled() {
    return is_enabled;
}

void ChromaticAberration::setEnabled(bool enable) {
    is_enabled = enable;
}

void ChromaticAberration::setChromaticAberration(double cc) {
    chromatic_aberration = cc;
}

double ChromaticAberration::chromaticAberration() {
    return chromatic_aberration;
}


double ChromaticAberration::getDistHeight(double sigma) {
    return 1.0 / (sigma * std::log(2.0 * Constants::Pi));
}

double ChromaticAberration::generateDist() {
    // To try and explain this:
    // I want a distribution with the shape of a gaussian (normal) distribution, but with a different sigma in the
    // positive and negative direction from the mean.
    // to do this, and have a continuous function, I use a uniform random variable to choose the percentage of each
    // distribution that I sample.

    double ratio = sigma_neg / sigma_pos;
    ratio = ratio / (ratio + 1.0);

    double selector = dist_selector(rng);
    double dist_1 = std::fabs(dist(rng));

    if (selector >= ratio)
        return dist_1 * sigma_pos;
    else
        return -1.0*(dist_1 * sigma_neg);
}

double ChromaticAberration::sigmaPositive() {
    return sigma_pos;
}

double ChromaticAberration::sigmaNegative() {
    return sigma_neg;
}

double ChromaticAberration::halfWidthHalfMaxPositive() {
    return 0.5 * sigma_pos * 2.0 * std::sqrt( 2.0 * std::log(2.0));
}

double ChromaticAberration::halfWidthHalfMaxNegative() {
    return 0.5 * sigma_neg * 2.0 * std::sqrt( 2.0 * std::log(2.0));
}


void ChromaticAberration::setSigmas(double sig_neg, double sig_pos) {
    setSigmaNegative(sig_neg);
    setSigmaPositive(sig_pos);
}

void ChromaticAberration::setSigmaPositive(double sig) {
    sigma_pos = std::fabs(sig);
}

void ChromaticAberration::setSigmaNegative(double sig) {
    sigma_neg = std::fabs(sig);
}

double ChromaticAberration::getFocusChange(double kilo_volts) {
    // cc in mm, kilo_volts in kilo electron volts
    // distribution in electron volts
    // want result in Angstroms
    // hance factor of 100

    return 100 * chromatic_aberration * generateDist() / kilo_volts;
}

void ChromaticAberration::setHalfWidthHalfMaxs(double hwhm_neg, double hwhm_pos) {
    setHalfWidthHalfMaxPositive(hwhm_pos);
    setHalfWidthHalfMaxNegative(hwhm_neg);
}

void ChromaticAberration::setHalfWidthHalfMaxPositive(double hwhm) {
    //hwhm to sigma
    setSigmaPositive(2.0 * hwhm / (2.0 * std::sqrt( 2.0 * std::log(2.0))));
}

void ChromaticAberration::setHalfWidthHalfMaxNegative(double hwhm) {
    setSigmaNegative(2.0 * hwhm / (2.0 * std::sqrt( 2.0 * std::log(2.0))));
}
