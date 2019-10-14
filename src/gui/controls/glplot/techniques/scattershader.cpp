//
// Created by Jon on 14/10/2019.
//

#include "scattershader.h"

namespace PGL{

    ScatterShader::ScatterShader() {
        _posBufLocation = 0xffffffff;
        _colBufLocation = 0xffffffff;
    }

    void ScatterShader::initialise() {
        Shader::initialise();

        compileFromFile(GL_VERTEX_SHADER, ":/PGL/Shaders/scatter.vs");

        compileFromFile(GL_GEOMETRY_SHADER, ":/PGL/Shaders/scatter.gs");

        compileFromFile(GL_FRAGMENT_SHADER, ":/PGL/Shaders/scatter.fs");

        finalise();

        _MVLocation = getUniformLocation("ModelView");
        _PLocation = getUniformLocation("Proj");

        if (_MVLocation == 0xffffffff || _PLocation == 0xffffffff)
            throw std::runtime_error("Error, failed to initialise uniform locations");

        _colBufLocation = getAttribLocation("ColBuf");
        _posBufLocation = getAttribLocation("PosBuf");

        if (_posBufLocation == 0xffffffff || _colBufLocation == 0xffffffff) {
            throw std::runtime_error("Error, failed to initialise buffer locations");
        }
    }
}