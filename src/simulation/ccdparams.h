//
// Created by jon on 02/04/18.
//

#ifndef CLTEM_CCDPARAMS_H
#define CLTEM_CCDPARAMS_H


#include <vector>
#include <mutex>
#include <algorithm>

class CCDParams
{
private:
    static std::vector<std::vector<double>> ntfs;

    static std::vector<std::vector<double>> dqes;

    static std::vector<std::string> names;

    static std::mutex mtx;

    static long getIndexFromName(std::string name)
    {
        long pos = std::find(names.begin(), names.end(), name) - names.begin();
        return pos;
    }

public:
    static bool nameExists(std::string name)
    {
        return std::find(names.begin(), names.end(), name) != names.end();
    }

    static void addCCD(std::string name, std::vector<double> dqe, std::vector<double> ntf)
    {
        std::lock_guard<std::mutex> lck(mtx);

        if (nameExists(name))
            throw std::runtime_error("Trying to load CCD: " + name + " but it already exists.");

        names.push_back(name);
        dqes.push_back(dqe);
        ntfs.push_back(ntf);
    }

    static std::vector<std::string> getNames()
    {
        std::lock_guard<std::mutex> lck(mtx);
        return names;
    }

    static std::vector<double> getDQE(std::string name) {
        std::lock_guard<std::mutex> lck(mtx);
        long ind = getIndexFromName(name);
        return dqes[ind];
    }

    static std::vector<double> getNTF(std::string name) {
        std::lock_guard<std::mutex> lck(mtx);
        long ind = getIndexFromName(name);
        return ntfs[ind];
    }

};


#endif //CLTEM_CCDPARAMS_H
