//
// Created by Jon on 14/10/2019.
//

#ifndef CLTEM_RECTANGLESHADER_H
#define CLTEM_RECTANGLESHADER_H

#include "shader.h"

namespace PGL {
    class RectangleShader : public PGL::Shader {
    public:
        RectangleShader();

        ~RectangleShader() = default;

        void initialise() override;

        GLint _posBufLocation, _colLocation, _minsLocation, _maxsLocation, _pixelSizeLocation;
    };
}

#endif //CLTEM_RECTANGLESHADER_H
