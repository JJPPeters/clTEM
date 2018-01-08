#ifndef CLTEM_STRUCTUREPARAMETERS_H
#define CLTEM_STRUCTUREPARAMETERS_H

#include <vector>
#include <mutex>

class StructureParameters
{
private:
    static std::vector<float> Params;

    static std::mutex mtx;
public:
    static void setParams(std::vector<float> p)
    {
        std::lock_guard<std::mutex> lck(mtx);
        Params = p;
    }

    static std::vector<float> getParams()
    {
        std::lock_guard<std::mutex> lck(mtx);
        return Params;
    }
};

#endif //CLTEM_STRUCTUREPARAMETERS_H
