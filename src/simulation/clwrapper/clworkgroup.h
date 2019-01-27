//
// Created by jon on 08/10/16.
//

#ifndef CLWRAPPER_MAIN_CLWORKGROUP_H
#define CLWRAPPER_MAIN_CLWORKGROUP_H

#include "CL/cl.hpp"

// Specifiy number of threads to launch
class clWorkGroup
{
public:
    clWorkGroup(unsigned int x, unsigned int y, unsigned int z)
    {
        worksize = cl::NDRange(x, y, z);
    };

    clWorkGroup(unsigned int x, unsigned int y)
    {
        worksize = cl::NDRange(x, y);
    };

    clWorkGroup(unsigned int x)
    {
        worksize = cl::NDRange(x);
    };

    cl::NDRange worksize;
};

#endif //CLWRAPPER_MAIN_CLWORKGROUP_H
