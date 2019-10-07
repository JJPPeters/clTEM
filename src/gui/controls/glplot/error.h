//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_ERROR_H
#define CLTEM_ERROR_H

#include <cstring>
#include <vector>
#include <string>
#include <stdexcept>

#include <GL/gl.h>
#include <GL/glext.h>

#define OGLCheckError() (glGetError() == GL_NO_ERROR)
#define ZERO_MEM(a) std::memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define RANDOM rand
#define INVALID_UNIFORM_LOCATION 0xffffffff

namespace PGL {
    std::string ErrorToString(GLenum err);

    void CheckErrors(std::string prefix = "");
}

#endif //CLTEM_ERROR_H
