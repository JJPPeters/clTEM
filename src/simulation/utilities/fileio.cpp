//
// Created by jon on 17/12/17.
//

#include <ccdparams.h>
#include "fileio.h"

namespace fileio {
    void SaveSettingsJson(std::string filepath, json settings) {
        std::ofstream fout(filepath);

        if (!fout.is_open())
            throw std::runtime_error("Could not open .json file for saving: " + filepath);

        fout << std::setw(4) << settings; // setw required to get nice indented output
        fout.close();
    }

    json OpenSettingsJson(std::string filepath) {
        std::ifstream in(filepath);

        if (!in.is_open())
            throw std::runtime_error("Could not open .json file for reading: " + filepath);

        json j;
        in >> j;

        return j;
    }
}