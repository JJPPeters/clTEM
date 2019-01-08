//
// Created by Jon on 08/01/2019.
//
#include "easylogging++.h"

#ifndef CLTEM_LOGGING_H
#define CLTEM_LOGGING_H

#if ELPP_COMPILER_GCC  // GCC (as this is what I use)
// https://stackoverflow.com/a/15775519
void initialiseLogging();

std::string getMethodName(const std::string& prettyFunction);

#undef ELPP_FUNC
#define ELPP_FUNC getMethodName(__PRETTY_FUNCTION__).c_str()

//#define TEST_FUNC getTestName(__PRETTY_FUNCTION__)

#endif

#endif //CLTEM_LOGGING_H
