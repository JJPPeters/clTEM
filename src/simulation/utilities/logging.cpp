//
// Created by Jon on 08/01/2019.
//

#include "logging.h"

INITIALIZE_EASYLOGGINGPP

#if ELPP_COMPILER_GCC

std::string getMethodName(const std::string& prettyFunction) {
    // find our '(' as we want to find the spaces not in the function list
    size_t l_bracket = prettyFunction.rfind('(');

    // find the last space before the bracket
    size_t begin = prettyFunction.substr(0, l_bracket).rfind(' ') + 1;

    return prettyFunction.substr(begin, l_bracket - begin);
}
#endif