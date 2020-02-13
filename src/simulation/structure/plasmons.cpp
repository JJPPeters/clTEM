//
// Created by Jon on 13/02/2020.
//

#include <utilities/stringutils.h>
#include <algorithm>
#include <utilities/vectorutils.h>
#include <utilities/commonstructs.h>
#include "plasmons.h"

void Plasmons::setSpecificPhononsFromString(std::string list_string) {
    std::vector<int> selection;

    if (list_string.empty())
        individual_plasmons = {0};

    // split our string by the commas
    std::vector<std::string> split_string = Utils::splitStringDelimiter(list_string, ',');

    if (split_string.empty())
        individual_plasmons = {0};

    // now process each part of the string
    for (auto &p_string : split_string) {
        // check we don't have just a hyphen(s)
        if (p_string.find_first_not_of('-') == std::string::npos)
            continue;

        // I'm assuming starting with a hyphen means start from 0
        // i.e. -9 means 0-9
        std::string part_string = p_string;
        if (Utils::stringBeginsWith(p_string, "-"))
            part_string = "0" + part_string;

        // I'm not sure about using multiple hyphens, I think I will allow it?
        // probably only go from first value to largest value?
        // first split by hyphens and convert to integers
        std::vector<std::string> split_part = Utils::splitStringDelimiter(part_string, '-');
        std::vector<int> split_part_int;
        for (auto &int_str : split_part) {
            split_part_int.push_back( std::stoi(int_str) );
        }


        if (split_part_int.size() == 1) {
            // handle case of only one number!!
            selection.push_back(split_part_int[0]);
        } else {
            // is vector sorted ascending or descending
            int start;
            int finish;
            if (std::is_sorted(split_part_int.begin(), split_part_int.end())) {
                // just use first/last value
                start = split_part_int.front();
                finish = split_part_int.back();
            } else if (std::is_sorted(split_part_int.rbegin(), split_part_int.rend())) {
                // just use last/first value
                start = split_part_int.back();
                finish = split_part_int.front();
            } else {
                // this becomes tricky!
                // I think I want to handle this using min/max values?
                start = *std::min_element(split_part_int.begin(), split_part_int.end());
                finish = *std::max_element(split_part_int.begin(), split_part_int.end());
            }

            for (int i = start; i <= finish; i++)
                selection.push_back(i);

        }

        // I'm ignoring hyphens at the end (there is a possibility to make this simulate all possible,
        // but this would be tricky to implement (and not really needed, just simulate 0-99999 would cover all cases)
    }

    // remove any duplicates now
    selection = Utils::removeDuplicates(selection);

    individual_plasmons = selection;
    individual_plasmons_valid = std::vector<bool>(selection.size(), false);
}

std::string Plasmons::getSpecificPhononsAsString() {
    std::string out = "";

    if (individual_plasmons.empty())
        return "0";

    // sort to be double sure
    std::sort(individual_plasmons.begin(), individual_plasmons.end());

    int start = individual_plasmons[0];

    for (int i = 0; i < individual_plasmons.size(); ++i) {
        int current_val = individual_plasmons[i];
        int next_val = 0;
        if (i+1 < individual_plasmons.size())
            next_val = individual_plasmons[i+1];

        if (next_val == current_val + 1)
            continue;
        else {
            if (start == current_val)
                out += std::to_string(start) + ",";
            else if (start + 1 == current_val)
                out += std::to_string(start) + "," + std::to_string(current_val) + ",";
            else
                out += std::to_string(start) + "-" + std::to_string(current_val) + ",";

            start = next_val;
        }
    }

    // need to remove the last comma
    return out.substr(0, out.size()-1);

}

double Plasmons::getScatteringDistance() {
    double rnd = dist(rng);
    return -1.0 * mean_free_path * std::log(rnd);
}

double Plasmons::getScatteringAzimuth() {
    double rnd = dist(rng);
    return 2.0 * Constants::Pi * rnd;
}

double Plasmons::getScatteringPolar() {
    double rnd = dist(rng);

    double crit_2 = critical_angle*critical_angle;
    double char_2 = characteristic_angle*characteristic_angle;

    double ang_2 = char_2 * (std::pow((crit_2 + char_2) / char_2, rnd) - 1);
    return std::sqrt(ang_2);
}

void Plasmons::getDistancesForIndividual(int configurations, double thickness) {
    // This works out the scattering depths for the number of plasmon events required within the thickness

    // The main difficulty is to require the program to exit properly if the user has put some stupid number
    // i.e. the 10 billionth plasmon in a graphene structure...

    // to save time, I do all the plasmon numbers at once!
    // many nested vectors
    // top level is the plasmon number
    // middle level is the configurations requested!
    // bottom level is the actual depths of the scattering!

    // I know the sizes of all these vectors, so I can initialise them here!
    std::vector<std::vector<std::vector<double>>> output(individual_plasmons.size());
    for (int i = 0; i < individual_plasmons.size(); ++i) {
        std::vector<std::vector<double>> configs(configurations);
        for (int j = 0; j < configs.size(); ++j)
            configs[j] = std::vector<double>(individual_plasmons[i]);
        output[i] = configs;
    }

    int max_plasmon = individual_plasmons.back();

    // this is where I store how many configurations I have
    std::vector<int> counts(individual_plasmons.size(), 0);
    unsigned int attempt_limit = individual_plasmons.size() * max_plasmon * configurations * 1000000;
    for (unsigned int attempt_count = 0; attempt_count < attempt_limit; ++attempt_count) {

        double current_t = thickness;
        std::vector<double> depths;
        depths.reserve(max_plasmon);
        int c = 0;

        while (current_t > 0.0 && c < max_plasmon) {
            double d = getScatteringDistance();
            depths.push_back(d);
            current_t -= d;
        }

        int current_plasmon = depths.size();

        //convert this plasmon to an index (or detect that it is not desired
        auto current_iterator = std::find(individual_plasmons.begin(), individual_plasmons.end(), current_plasmon);
        if (current_iterator == individual_plasmons.end())
            continue;
        auto current_i = std::distance(individual_plasmons.begin(), current_iterator);
        if (counts[current_i] <= configurations) {
            output[current_i][counts[current_i]] = depths;
            counts[current_i] += 1;
        }
    }

    // now we just need to work out what parts haven't got the required configuration count
    // could just remove them, but it is probably easier to set a flag to say if they are valid or not!
    for (int i = 0; i < individual_plasmons.size(); ++i) {
        if (counts[i] == configurations)
            individual_plasmons_valid[i] = true;
    }
}
