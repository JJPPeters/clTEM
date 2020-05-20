//
// Created by Jon on 13/02/2020.
//

#ifndef CLTEM_PLASMON_H
#define CLTEM_PLASMON_H


#include <vector>
#include <string>
#include <random>
#include <chrono>

#define THICKNESS_IT_LIMIT 1e10
#define INDIVIDUAL_IT_LIMIT 1e10

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
        plasmons_enabled = false;

        mean_free_path = 10;
        characteristic_angle = 1;
        critical_angle = 0.1;

        simulate_combined_plasmons = true;

        simulate_individual_plasmon = false;
        individual_plasmon = 1;

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
        simulate_individual_plasmon = !do_sim;
    }
    void setIndividualEnabled(bool do_sim) {
        simulate_individual_plasmon = do_sim;
        simulate_combined_plasmons = !do_sim;
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
    std::vector<double> getDistancesforCombined(double thickness);

    double getGeneratedDepth(unsigned int job_id, unsigned int scattering_count);
    std::vector<std::vector<unsigned int>> getPlasmonNumbers();
};


#endif //CLTEM_PLASMON_H
