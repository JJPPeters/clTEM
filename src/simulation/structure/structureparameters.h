#ifndef CLTEM_STRUCTUREPARAMETERS_H
#define CLTEM_STRUCTUREPARAMETERS_H

#include <vector>
#include <mutex>
#include <algorithm>

class StructureParameters
{
private:
    static std::vector<std::string> Names;

    static std::vector<std::vector<float>> Params;

    static std::mutex mtx;

    static int Current;

public:
    static void setParams(std::vector<float> p, std::string name)
    {
        std::lock_guard<std::mutex> lck(mtx);
        // test if the name already exists
        if (std::find(Names.begin(), Names.end(), name)!= Names.end())
            throw std::runtime_error("Error adding duplicate parameter file");

        Names.push_back(name);

        Params.push_back(p);

        if (Params.size() == 1)
            Current = 0;
    }

    static std::vector<float> getParams()
    {
        std::lock_guard<std::mutex> lck(mtx);
        return Params[Current];
    }

    static void setCurrent(std::string name)
    {
        std::lock_guard<std::mutex> lck(mtx);
        auto ind = std::find(Names.begin(), Names.end(), name);
        if (ind == Names.end())
            return;

        Current = (int) (ind - Names.begin());
    }

    static std::vector<std::string> getNames() {std::lock_guard<std::mutex> lck(mtx); return Names;}
    static int getCurrent() {std::lock_guard<std::mutex> lck(mtx); return Current;}

    static std::string getCurrentName() {std::lock_guard<std::mutex> lck(mtx); return Names[Current];}
};

#endif //CLTEM_STRUCTUREPARAMETERS_H
