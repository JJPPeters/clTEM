#ifndef CLTEM_STRUCTUREPARAMETERS_H
#define CLTEM_STRUCTUREPARAMETERS_H

#include <vector>
#include <mutex>
#include <algorithm>

// This class is purely static, and is mainly used for reading in the required data and storing it.
// When it is actually needed, it is copied to the SimulationManager classes so they have their own copy
// and this can be edited whilst a simulation is running...
class StructureParameters
{
private:
    static std::vector<std::string> Names;

    static std::vector<std::vector<float>> Params;

    static std::mutex mtx;

public:
    static void setParams(const std::vector<float> &p, const std::string &name) {
        std::lock_guard<std::mutex> lck(mtx);
        // test if the name already exists
        if (std::find(Names.begin(), Names.end(), name)!= Names.end())
            throw std::runtime_error("Error adding duplicate parameter file");

        Names.push_back(name);
        Params.push_back(p);
    }

    static std::vector<float> getParams(const std::string &name) {
        std::lock_guard<std::mutex> lck(mtx);

        auto ind = std::find(Names.begin(), Names.end(), name);
        if (ind == Names.end())
            return std::vector<float>();

        int Current = (int) (ind - Names.begin());

        return Params[Current];
    }


    static std::vector<std::string> getNames() {std::lock_guard<std::mutex> lck(mtx); return Names;}
};

#endif //CLTEM_STRUCTUREPARAMETERS_H
