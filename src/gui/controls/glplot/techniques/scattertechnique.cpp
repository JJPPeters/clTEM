//
// Created by Jon on 03/09/2018.
//

#include <iostream>
#include "scattertechnique.h"

namespace PGL {

    Scatter::Scatter()
    {
        _posBufLocation = 0xffffffff;
        _colBufLocation = 0xffffffff;
        _haveBuffers = false;
    }

    Scatter::Scatter(std::vector<Vector3f> pos, std::vector<Vector3f> col)
    {
        _posBufLocation = 0xffffffff;
        _colBufLocation = 0xffffffff;
        _haveBuffers = false;

        Init();

        MakeBuffers(pos, col);
    }

    void Scatter::Init() {
        Technique::Init();

//        PGL::CheckErrors("OpenGL: BillBoard: Initialising technique");

        CompileShaderFromFile(GL_VERTEX_SHADER, ":/PGL/Shaders/scatter.vs");

//        PGL::CheckErrors("OpenGL: BillBoard: Creating vertex shader");

        CompileShaderFromFile(GL_GEOMETRY_SHADER, ":/PGL/Shaders/scatter.gs");

//        PGL::CheckErrors("OpenGL: BillBoard: Creating geometry shader");

        CompileShaderFromFile(GL_FRAGMENT_SHADER, ":/PGL/Shaders/scatter.fs");

//        PGL::CheckErrors("OpenGL: BillBoard: Creating fragment shader");

        Finalise();

//        PGL::CheckErrors("OpenGL: BillBoard: Finalising technique");

        _MVLocation = GetUniformLocation("ModelView");
        _PLocation = GetUniformLocation("Proj");

        if (_MVLocation == 0xffffffff || _PLocation == 0xffffffff)
            throw std::runtime_error("Error, failed to initialise uniform locations");

        _colBufLocation = GetAttribLocation("ColBuf");
        _posBufLocation = GetAttribLocation("PosBuf");

        if (_posBufLocation == 0xffffffff || _colBufLocation == 0xffffffff) {
            throw std::runtime_error("Error, failed to initialise buffer locations");
        }
    }

    void Scatter::MakeBuffers(std::vector<Vector3f>& positions, std::vector<Vector3f>& colours)
    {
        _haveBuffers = false;

        for (auto &p: positions) {
            if (p[0] < _limits(0, 0))
                _limits(0, 0) = p[0];
            if (p[0] > _limits(0, 1))
                _limits(0, 1) = p[0];
            // y
            if (p[1] < _limits(1, 0))
                _limits(1, 0) = p[1];
            if (p[1] > _limits(1, 1))
                _limits(1, 1) = p[1];
            // z
            if (p[2] < _limits(2, 0))
                _limits(2, 0) = p[2];
            if (p[2] > _limits(2, 1))
                _limits(2, 1) = p[2];
        }

        _colourBuffer = std::make_shared<AttributeBuffer>(AttributeBuffer(colours, static_cast<GLuint>(_colBufLocation)));
        _positionBuffer = std::make_shared<AttributeBuffer>(AttributeBuffer(positions, static_cast<GLuint>(_posBufLocation)));
        _haveBuffers = true;
    }

    void Scatter::Render(const Matrix4f &MV, const Matrix4f &P, float pix_size)
    {

        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        Enable();
        SetModelView(MV);
        SetProj(P);

        _colourBuffer->Bind();
        _positionBuffer->Bind();

        glFuncs->glDrawArrays(GL_POINTS, 0, _positionBuffer->getSize());

        _colourBuffer->Unbind();
        _positionBuffer->Unbind();

        Disable();
    }

}