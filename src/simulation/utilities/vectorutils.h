//
// Created by Jon on 15/05/2018.
//

#ifndef CLTEM_VECTORUTILS_H
#define CLTEM_VECTORUTILS_H


#include <vector>
#include <algorithm>

namespace Utils
{
    template <typename T>
    int findItemIndex(std::vector<T>& vec, T val)
    {
        ptrdiff_t pos = std::distance(vec.begin(), std::find(vec.begin(), vec.end(), val));
        if(pos >= vec.size()) {
            return -1;
        }

        return static_cast<int>(pos);
    }

}


#endif //CLTEM_VECTORUTILS_H
