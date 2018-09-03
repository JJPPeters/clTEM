//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLDEFINES_H
#define CLTEM_OGLDEFINES_H

#include <cstring>

#define OGLCheckError() (glGetError() == GL_NO_ERROR)
#define ZERO_MEM(a) std::memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define RANDOM rand
#define INVALID_UNIFORM_LOCATION 0xffffffff


#endif //CLTEM_OGLDEFINES_H
