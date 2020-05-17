//
// Created by Jon on 13/02/2020.
//

#ifndef CLTEM_PLASMON_H
#define CLTEM_PLASMON_H


#include <vector>
#include <string>
#include <random>
#include <chrono>


class PlasmonScattering {
private:
    // units of nm
    double mean_free_path;

    // units of mrad
    double characteristic_angle;
    double critical_angle;

    // this is an overrinding 'enabled/disabled' toggle
    // allows us to store other parameters even when we don't want to activate them
    bool plasmons_enabled;

    // TODO: could combine these into one enum for the type?
    // data for simulating combined plasmon image
    bool simulate_combined_plasmons;

    // data for selected plasmons
    bool simulate_individual_plasmon;
    unsigned int individual_plasmon;

    std::mt19937_64 rng;
    std::uniform_real_distribution<> dist;

    std::vector<std::vector<double>> depths;

public:

    PlasmonScattering() {
        mean_free_path = 0;
        characteristic_angle = 0;
        critical_angle = 0;

        simulate_combined_plasmons = false;

        simulate_individual_plasmon = false;
        individual_plasmon = 0;

        dist = std::uniform_real_distribution<>(0, 1);
        rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());
    }

    void setEnabled(bool enabled) {plasmons_enabled = enabled;}
    bool getPlasmonEnabled() {return plasmons_enabled;}
//    bool getPlasmonEnabled() {return simulate_combined_plasmons || (simulate_individual_plasmon && individual_plasmon > 0);}

    void setMeanFreePath(double mfp){mean_free_path = mfp;}
    void setCharacteristicAngle(double angle) {characteristic_angle = angle;}
    void setCriticalAngle(double angle) {critical_angle = angle;}
    void setIndividualPlasmon(unsigned int n) {individual_plasmon = n;}
    void setCombinedEnabled(bool do_sim) {
        simulate_combined_plasmons = do_sim;
        if (do_sim && simulate_individual_plasmon)
            simulate_individual_plasmon = false;
    }
    void setIndividualEnabled(bool do_sim) {
        simulate_individual_plasmon = do_sim;
        if (do_sim && simulate_combined_plasmons)
            simulate_combined_plasmons = false;
    }

    double getMeanFreePath(){return mean_free_path;}
    double getCharacteristicAngle() {return characteristic_angle;}
    double getCriticalAngle() {return critical_angle;}
    unsigned int getIndividualPlasmon() {return individual_plasmon;}
    bool getCombinedEnabled() {return simulate_combined_plasmons;}
    bool getIndividualEnabled() {return simulate_individual_plasmon;}

    double getScatteringDistance();
    double getScatteringAzimuth();
    double getScatteringPolar();


    void initDepthVectors(unsigned int job_count);
    bool generateScatteringDepths(unsigned int job_id, double thickness);
    std::vector<double> getDistancesForIndividual(double thickness);
};


#endif //CLTEM_PLASMON_H
