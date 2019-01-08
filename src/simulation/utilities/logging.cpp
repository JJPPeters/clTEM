//
// Created by Jon on 08/01/2019.
//

#include "logging.h"

//INITIALIZE_EASYLOGGINGPP

#if ELPP_COMPILER_GCC

INITIALIZE_EASYLOGGINGPP

std::string getMethodName(const std::string& prettyFunction) {
    size_t begin = prettyFunction.find(' ') + 1;
    if (begin > prettyFunction.find("::"))
        begin = 0;
    size_t len = prettyFunction.rfind('(') - begin;

    return prettyFunction.substr(begin, len);
}
#endif