//
// Created by jonat on 24/05/2020.
//

#ifndef CLTEM_CHROMATICABERRATION_H
#define CLTEM_CHROMATICABERRATION_H

#include <random>
#include <chrono>

class ChromaticAberration {
private:

    bool is_enabled;

    // stored in mm
    double chromatic_aberration;

    double sigma_neg;
    double sigma_pos;

    std::mt19937_64 rng;
    std::normal_distribution<> dist;
    std::uniform_real_distribution<> dist_selector;

    double getDistHeight(double sigma);

    double generateDist();

public:

    ChromaticAberration();

    bool enabled();
    void setEnabled(bool enable);

    double chromaticAberration();
    void setChromaticAberration(double cc);

    double sigmaPositive();
    double sigmaNegative();

    double halfWidthHalfMaxPositive();
    double halfWidthHalfMaxNegative();

    void setSigmas(double sig_neg, double sig_pos);
    void setSigmaPositive(double sig);
    void setSigmaNegative(double sig);

    void setHalfWidthHalfMaxs(double hwhm_neg, double hwhm_pos);
    void setHalfWidthHalfMaxPositive(double hwhm);
    void setHalfWidthHalfMaxNegative(double hwhm);

    double getFocusChange(double kilo_volts);

};


#endif //CLTEM_CHROMATICABERRATION_H
