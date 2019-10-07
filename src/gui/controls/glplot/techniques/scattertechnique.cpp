//
// Created by Jon on 03/09/2018.
//

#include <iostream>
#include "scattertechnique.h"

#include <limits>

namespace PGL {

    Scatter::Scatter()
    {
        _posBufLocation = 0xffffffff;
        _colBufLocation = 0xffffffff;
        _haveBuffers = false;

        _limits << std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
                std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
                std::numeric_limits<float>::min(), std::numeric_limits<float>::max();
//        Q_INIT_RESOURCE(shaders);
    }

    Scatter::Scatter(std::vector<Vector3f> pos, std::vector<Vector3f> col)
    {
        _posBufLocation = 0xffffffff;
        _colBufLocation = 0xffffffff;
        _haveBuffers = false;

        _limits << std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
                std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
                std::numeric_limits<float>::min(), std::numeric_limits<float>::max();
        MakeBuffers(pos, col);
    }

    void Scatter::Init() {
        Technique::Init();

    //    OGLCheckErrors("OpenGL: BillBoard: Initialising technique");

        CompileShaderFromFile(GL_VERTEX_SHADER, ":/OGL/Shaders/billboard.vs");

    //    OGLCheckErrors("OpenGL: BillBoard: Creating vertex shader");

        CompileShaderFromFile(GL_GEOMETRY_SHADER, ":/OGL/Shaders/billboard.gs");

    //    OGLCheckErrors("OpenGL: BillBoard: Creating geometry shader");

        CompileShaderFromFile(GL_FRAGMENT_SHADER, ":/OGL/Shaders/billboard.fs");

    //    OGLCheckErrors("OpenGL: BillBoard: Creating fragment shader");

        Finalise();

    //    OGLCheckErrors("OpenGL: BillBoard: Finalising technique");

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

    void Scatter::Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize)
    {
        if (!_haveBuffers)
            return;

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