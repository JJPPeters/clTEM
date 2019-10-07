//
// Created by Jon on 03/09/2018.
//

#include <iostream>
#include "rectangletechnique.h"

namespace PGL {

    RectangleTechnique::RectangleTechnique() {
        _haveBuffers = false;
        Q_INIT_RESOURCE(shaders);
    }

    void RectangleTechnique::Init() {
        Technique::Init();

//    OGLCheckErrors("OpenGL: Rectangle: Initialising technique");

        CompileShaderFromFile(GL_VERTEX_SHADER, ":/OGL/Shaders/rectangle.vs");

//    OGLCheckErrors("OpenGL: Rectangle: Creating vertex shader");

        CompileShaderFromFile(GL_FRAGMENT_SHADER, ":/OGL/Shaders/rectangle.fs");

//    OGLCheckErrors("OpenGL: Rectangle: Creating fragment shader");

        Finalise();

//    OGLCheckErrors("OpenGL: Rectangle: Finalising technique");

//    _MVLocation = GetUniformLocation("ModelView");
//    _PLocation = GetUniformLocation("Proj");
        _minsLocation = GetUniformLocation("RectMins");
        _maxsLocation = GetUniformLocation("RectMaxs");
        _colLocation = GetUniformLocation("RectCol");

//    if (_MVLocation == 0xffffffff || _PLocation == 0xffffffff || _minsLocation == 0xffffffff || _maxsLocation == 0xffffffff || _colLocation == 0xffffffff)
//        throw std::runtime_error("OpenGL: Rectangle: Failed to initialise uniform locations");

        _posBufLocation = GetAttribLocation("PosBuf");

        if (_posBufLocation == 0xffffffff)
            throw std::runtime_error("OpenGL: Rectangle: Failed to initialise buffer locations");
    }

    void
    RectangleTechnique::MakeBuffers(std::vector<Vector3f> &positions, Vector4f &col, Vector3f &mins, Vector3f &maxs) {
        _haveBuffers = false;
        _positionBuffer = std::make_shared<AttributeBuffer>(
                AttributeBuffer(positions, static_cast<GLuint>(_posBufLocation)));
        std::vector<GLuint> els = {0, 1, 2, 2, 3, 0}; // TODO: this could be user defined...
        _indexBuffer = std::make_shared<ArrayBuffer>(ArrayBuffer(els, GL_ELEMENT_ARRAY_BUFFER));

        // TODO: this should be set when the square is defined
        _mins = mins;
        _maxs = maxs;
        _col = col;

        _haveBuffers = true;
    }

    void RectangleTechnique::Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize) {
        if (!_haveBuffers)
            return;

        Enable();
//    SetModelView(MV);
//    SetProj(P);
        SetLims(_mins, _maxs);
        SetCol(_col);

//        _positionBuffer->DrawArrays(false);
//        _indexBuffer->DrawElements(true, GL_TRIANGLES);

        Disable();
    }

    void RectangleTechnique::SetLims(const Vector3f &mins, const Vector3f &maxs) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glUniform3f(_minsLocation, mins.x, mins.y, mins.z);
        glFuncs->glUniform3f(_maxsLocation, maxs.x, maxs.y, maxs.z);
    }

    void RectangleTechnique::SetCol(const Vector4f &col) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glUniform4f(_colLocation, col.x, col.y, col.z, col.w);
    }

}