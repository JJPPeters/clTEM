//
// Created by Jon on 03/09/2018.
//

#include <iostream>
#include "rectangletechnique.h"



namespace PGL {

    RectangleTechnique::RectangleTechnique(float t, float l, float b, float r, float z, Vector4f &colour, PGL::Plane pl) {
        _haveBuffers = false;

        Init();

        std::vector<Vector3f> pos;

        if (pl == PGL::Plane::x)
            pos = {Vector3f(z, l, t), Vector3f(z, l, b), Vector3f(z, r, b), Vector3f(z, r, t)};
        else if (pl == PGL::Plane::y)
            pos = {Vector3f(l, z, t), Vector3f(l, z, b), Vector3f(r, z, b), Vector3f(r, z, t)};
        else if (pl == PGL::Plane::z)
            pos = {Vector3f(l, t, z), Vector3f(l, b, z), Vector3f(r, b, z), Vector3f(r, t, z)};

        MakeBuffers(pos, colour, pos[0], pos[2]);
    }

    void RectangleTechnique::Init() {
        Technique::Init();

//    OGLCheckErrors("OpenGL: Rectangle: Initialising technique");

        CompileShaderFromFile(GL_VERTEX_SHADER, ":/PGL/Shaders/rectangle.vs");

//    OGLCheckErrors("OpenGL: Rectangle: Creating vertex shader");

        CompileShaderFromFile(GL_FRAGMENT_SHADER, ":/PGL/Shaders/rectangle.fs");

//    OGLCheckErrors("OpenGL: Rectangle: Creating fragment shader");

        Finalise();

//    OGLCheckErrors("OpenGL: Rectangle: Finalising technique");

        _MVLocation = GetUniformLocation("ModelView");
        _PLocation = GetUniformLocation("Proj");
        _minsLocation = GetUniformLocation("RectMins");
        _maxsLocation = GetUniformLocation("RectMaxs");
        _colLocation = GetUniformLocation("RectCol");

        if (_MVLocation == 0xffffffff || _PLocation == 0xffffffff || _minsLocation == 0xffffffff || _maxsLocation == 0xffffffff || _colLocation == 0xffffffff)
            throw std::runtime_error("OpenGL: Rectangle: Failed to initialise uniform locations");

        _posBufLocation = GetAttribLocation("PosBuf");

        if (_posBufLocation == 0xffffffff)
            throw std::runtime_error("OpenGL: Rectangle: Failed to initialise buffer locations");
    }

    void
    RectangleTechnique::MakeBuffers(std::vector<Vector3f> &positions, Vector4f &col, Vector3f &mins, Vector3f &maxs) {
        _haveBuffers = false;
        _positionBuffer = std::make_shared<AttributeBuffer>(positions, static_cast<GLuint>(_posBufLocation));
        std::vector<unsigned int> els = {0, 1, 2, 2, 3, 0}; // TODO: this could be user defined...
        _indexBuffer = std::make_shared<ArrayBuffer>(els, GL_ELEMENT_ARRAY_BUFFER);

        // TODO: this should be set when the square is defined
        _mins = mins;
        _maxs = maxs;
        _col = col;

        _haveBuffers = true;
    }

    void RectangleTechnique::Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        Enable();
        SetModelView(MV);
        SetProj(P);

        SetLims(_mins, _maxs);
        SetCol(_col);

        _positionBuffer->Bind();
        _indexBuffer->Bind();

        glFuncs->glDrawElements(GL_TRIANGLES, 6, _indexBuffer->getType(), nullptr);

        _positionBuffer->Unbind();
        _indexBuffer->Unbind();

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