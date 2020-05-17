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
    std::vector<std::vector<double>> v(job_count, std::vector<double>(individual_plasmon));
    depths = v;
}

bool PlasmonScattering::generateScatteringDepths(unsigned int job_id, double thickness) {
    auto ds = getDistancesForIndividual(thickness);

    // TODO: test if valid

    depths[job_id] = ds;
    return true;
}

std::vector<double> PlasmonScattering::getDistancesForIndividual(double thickness) {
    // This works out the scattering depths for the number of plasmon events required within the thickness

    // The main difficulty is to require the program to exit properly if the user has put some stupid number
    // i.e. the 10 billionth plasmon in a graphene structure...

    // to save time, I do all the plasmon numbers at once!
    // many nested vectors
    // top level is the plasmon number
    // middle level is the configurations requested!
    // bottom level is the actual depths of the scattering!

    // I know the sizes of all these vectors, so I can initialise them here!

    std::vector<double> output(individual_plasmon);

    // this is where I store how many configurations I have
    unsigned int attempt_limit = individual_plasmon * 1000000;
    for (unsigned int attempt_count = 0; attempt_count < attempt_limit; ++attempt_count) {

        std::vector<double> depths;
        depths.reserve(individual_plasmon);
        double current_t = 0.0;
        unsigned int c = 0;

        // Loop through and get depths of scattering
        while (current_t <= thickness && c < individual_plasmon) {
            double d = getScatteringDistance();
            current_t += d;
            if (current_t <= thickness) {
                depths.push_back(current_t);
                ++c;
            }
        }

        // if we got what we wanted, set the depths in our output
        if (depths.size() == individual_plasmon) {
            output = depths;
            break;
        }
    }

    return output;
}
