//
// Created by Jon on 14/10/2019.
//

#ifndef CLTEM_SCATTERSHADER_H
#define CLTEM_SCATTERSHADER_H

#include "shader.h"
#include "attributebuffer.h"

namespace PGL {
class ScatterShader : public PGL::Shader {
    public:
        ScatterShader();

        ~ScatterShader() = default;

        void initialise() override;

        GLint _posBufLocation, _colBufLocation;

    };
}

#endif //CLTEM_SCATTERSHADER_H
