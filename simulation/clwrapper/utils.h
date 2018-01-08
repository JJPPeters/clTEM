//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_UTILS_H
#define CLWRAPPER_MAIN_UTILS_H

#include <string>

namespace Utils
{
    // Removes extra whitespace from OpenCL names for some devices/platforms
    std::string Trim(const std::string& str, const std::string& whitespace = " \t");
}

#endif //CLWRAPPER_MAIN_UTILS_H
