//
// Created by Jon on 14/10/2019.
//

#include "rectangleshader.h"

namespace PGL{

    RectangleShader::RectangleShader() {
        _posBufLocation = 0xffffffff;
        _colLocation = 0xffffffff;
        _minsLocation = 0xffffffff;
        _maxsLocation = 0xffffffff;
        _pixelSizeLocation = 0xffffffff;
    }

    void RectangleShader::initialise() {
        Shader::initialise();

        compileFromFile(GL_VERTEX_SHADER, ":/PGL/Shaders/rectangle.vs");

        compileFromFile(GL_FRAGMENT_SHADER, ":/PGL/Shaders/rectangle.fs");

        finalise();

        _MVLocation = getUniformLocation("ModelView");
        _PLocation = getUniformLocation("Proj");
        _minsLocation = getUniformLocation("RectMins");
        _maxsLocation = getUniformLocation("RectMaxs");

        _pixelSizeLocation = getUniformLocation("pixel_size");

        _colLocation = getUniformLocation("RectCol");

        if (_MVLocation == 0xffffffff || _PLocation == 0xffffffff || _colLocation == 0xffffffff
            || _minsLocation == 0xffffffff || _maxsLocation == 0xffffffff || _pixelSizeLocation == 0xffffffff)
            throw std::runtime_error("OpenGL: Rectangle: Failed to initialise uniform locations");

        _posBufLocation = getAttribLocation("PosBuf");

        if (_posBufLocation == 0xffffffff)
            throw std::runtime_error("OpenGL: Rectangle: Failed to initialise buffer locations");
    }

}