//
// Created by Jon on 13/02/2020.
//

#include <utilities/stringutils.h>
#include <algorithm>
#include <utilities/vectorutils.h>
#include <utilities/commonstructs.h>
#include "plasmon.h"

double PlasmonScattering::getScatteringDistance() {
    double rnd = dist(rng);
    return -1.0 * mean_free_path * std::log(rnd);
}

double PlasmonScattering::getScatteringAzimuth() {
    double rnd = dist(rng);
    return 2.0 * Constants::Pi * rnd;
}

double PlasmonScattering::getScatteringPolar() {
    double rnd = dist(rng);

    double crit_2 = critical_angle*critical_angle;
    double char_2 = characteristic_angle*characteristic_angle;

    double ang_2 = char_2 * (std::pow((crit_2 + char_2) / char_2, rnd) - 1);
    return std::sqrt(ang_2);
}

void PlasmonScattering::initDepthVectors(unsigned int job_count) {
    // Just create our vectors, so we don't waste time fucking around with memory later
    std::vector<std::vector<double>> v(job_count);
    depths = v;
}

bool PlasmonScattering::generateScatteringDepths(unsigned int job_id, double thickness) {
    if (job_id > depths.size())
        throw std::runtime_error("Error generating plasmon scattering depths, depth vectors may not have been initialised");

    std::vector<double> ds;

    if (simulate_combined_plasmons) {
        ds = getDistancesforCombined(thickness);

        if (ds.empty())
            return false;
    }
    else if (simulate_individual_plasmon) {
        ds = getDistancesForIndividual(thickness);

        if (ds.size() != individual_plasmon)
            return false;
    } else
        return false;

    depths[job_id] = ds;
    return true;
}

std::vector<double> PlasmonScattering::getDistancesForIndividual(double thickness) {
    // This works out the scattering depths for the number of plasmon events required within the thickness
    // It is basically a list of depths, and we keep generating them until they give the number of scattering events
    // that we want.

    std::vector<double> output;

    // this is where I store how many configurations I have
    unsigned int attempt_limit = individual_plasmon * INDIVIDUAL_IT_LIMIT;
    for (unsigned int attempt_count = 0; attempt_count < attempt_limit; ++attempt_count) {

        // TODO: this could be optimised to quit as soon as we know it will be larger
        auto depths = getDistancesforCombined(thickness);

        if (depths.size() == individual_plasmon) {
            output = depths;
            break;
        }
    }

    return output;
}

std::vector<double> PlasmonScattering::getDistancesforCombined(double thickness) {
    // this function will have varying length vectors, so we can't set them now
    std::vector<double> depths;
    depths.reserve(static_cast<int>(thickness / mean_free_path));

    double current_t = getScatteringDistance();
    unsigned int c = 0;

    // have a limit in case an absurd thickness or mean free path gets through
    while (current_t <= thickness && c < THICKNESS_IT_LIMIT) {
        depths.push_back(current_t);
        current_t += getScatteringDistance();
        c++;
    }

    if (c >= THICKNESS_IT_LIMIT)
        return std::vector<double>();
    else
        return depths;
}

double PlasmonScattering::getGeneratedDepth(unsigned int job_id, unsigned int scattering_count) {
    if (job_id > depths.size())
        throw std::runtime_error("Error generating plasmon scattering depths, depth vectors may not have been initialised");

    int total_scatters = depths[job_id].size();

    if (scattering_count == total_scatters)
        // this means we have finished our scattering
        // but we may have more material to propagate through, so return a sensible value
        return std::numeric_limits<double>::infinity();
    else if (scattering_count < total_scatters)
        return depths[job_id][scattering_count];
    else
        throw std::runtime_error("Error accessing scattering depths that do not exist");

}
