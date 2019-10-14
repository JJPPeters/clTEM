//
// Created by Jon on 03/09/2018.
//

#include "technique.h"

#include <iostream>

namespace PGL {
    Technique::Technique(bool visible) {
        _limits << std::numeric_limits<float>::max(), std::numeric_limits<float>::min(),
                std::numeric_limits<float>::max(), std::numeric_limits<float>::min(),
                std::numeric_limits<float>::max(), std::numeric_limits<float>::min();

        _visible = visible;
    }


}
