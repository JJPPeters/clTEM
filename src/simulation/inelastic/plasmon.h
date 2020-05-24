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

enum class PlasmonType {
    Full,
    Individual};

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

    // are we simulating an individual plasmon or all of them
    PlasmonType plasmon_sim_type;

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

        plasmon_sim_type = PlasmonType::Full;
        individual_plasmon = 1;

        dist = std::uniform_real_distribution<>(0, 1);
        rng = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());
    }

    bool enabled() {return plasmons_enabled;}
    void setEnabled(bool enabled) {plasmons_enabled = enabled;}

    double meanFreePath(){return mean_free_path;}
    void setMeanFreePath(double mfp){mean_free_path = mfp;}

    double characteristicAngle() {return characteristic_angle;}
    void setCharacteristicAngle(double angle) {characteristic_angle = angle;}

    double criticalAngle() {return critical_angle;}
    void setCriticalAngle(double angle) {critical_angle = angle;}

    unsigned int individualPlasmon() {return individual_plasmon;}
    void setIndividualPlasmon(unsigned int n) {individual_plasmon = n;}

    PlasmonType simType() {return plasmon_sim_type;}
    void setSimType(PlasmonType sim_type) {
        plasmon_sim_type = sim_type;
    }

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
