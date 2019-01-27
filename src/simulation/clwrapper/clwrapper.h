//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_CLWRAPPER_H
#define CLWRAPPER_MAIN_CLWRAPPER_H

//#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#define CL_HPP_MINIMUM_OPENCL_VERSION 100
#define CL_HPP_TARGET_OPENCL_VERSION 100

#include "CL/cl.hpp"

#include "clstatic.h"
#include "clcontext.h"
#include "clfourier.h"

#include "clmemory.h"
#include "clkernel.h"

#include "auto.h"

#endif //CLWRAPPER_MAIN_CLWRAPPER_H
