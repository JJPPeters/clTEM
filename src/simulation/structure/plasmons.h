//
// Created by Jon on 13/02/2020.
//

#ifndef CLTEM_PLASMONS_H
#define CLTEM_PLASMONS_H


#include <vector>
#include <string>
#include <random>
#include <chrono>


class Plasmons {
private:
    // units of nm
    double mean_free_path;

    // units of mrad
    double characteristic_angle;
    double critical_angle;

    // data for simulating combined plasmon image
    bool simulate_combined_plasmons;
    int combined_plasmon_iterations;

    // data for selected plasmons
    bool simulate_individual_plasmons;
    std::vector<int> individual_plasmons;
    std::vector<bool> individual_plasmons_valid;

    std::mt19937_64 rng;
    std::uniform_real_distribution<> dist;

public:

    Plasmons() {
        mean_free_path = 0;
        characteristic_angle = 0;
        critical_angle = 0;

        simulate_combined_plasmons = false;
        combined_plasmon_iterations = 1;

        simulate_individual_plasmons = false;
        individual_plasmons = {0};

        dist = std::uniform_real_distribution<>(0, 1);
        rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());
    }

    void setSpecificPhononsFromString(std::string list_string);
    void setMeanFreePath(double mfp){mean_free_path = mfp;}
    void setCharacteristicAngle(double angle) {characteristic_angle = angle;}
    void setCriticalAngle(double angle) {critical_angle = angle;}
    void setSimulateCombined(bool do_sim) {simulate_combined_plasmons = do_sim;}
    void setSimulateIndividual(bool do_sim) {simulate_individual_plasmons = do_sim;}
    void setCombinedPlasmonIterations(int iterations) {combined_plasmon_iterations = iterations;}

    std::string getSpecificPhononsAsString();
    double getMeanFreePath(){return mean_free_path;}
    double getCharacteristicAngle() {return characteristic_angle;}
    double getCriticalAngle() {return critical_angle;}
    bool getSimulateCombined() {return simulate_combined_plasmons;}
    bool getSimulateIndividual() {return simulate_individual_plasmons;}
    int getCombinedPlasmonIterations() {return combined_plasmon_iterations;}

    double getScatteringDistance();
    double getScatteringAzimuth();
    double getScatteringPolar();

    void getDistancesForIndividual(int configurations, double thickness);
};


#endif //CLTEM_PLASMONS_H
