//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLSTATIC_H
#define CLTEM_OGLSTATIC_H

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

std::string OGLErrorToString(GLenum err);

void OGLCheckErrors(std::string prefix = "");


#endif //CLTEM_OGLSTATIC_H
