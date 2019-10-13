//
// Created by Jon on 03/09/2018.
//

#include <iostream>
#include "rectangletechnique.h"



namespace PGL {

    Rectangle::Rectangle(float t, float l, float b, float r, float z, Vector4f &colour, PGL::Plane pl) : Technique() {
        _haveBuffers = false;

        Init();

        std::vector<Vector3f> pos;

        if (pl == PGL::Plane::x)
            pos = {Vector3f(z, l, t), Vector3f(z, l, b), Vector3f(z, r, b), Vector3f(z, r, t)};
        else if (pl == PGL::Plane::y)
            pos = {Vector3f(l, z, t), Vector3f(l, z, b), Vector3f(r, z, b), Vector3f(r, z, t)};
        else if (pl == PGL::Plane::z)
            pos = {Vector3f(l, t, z), Vector3f(l, b, z), Vector3f(r, b, z), Vector3f(r, t, z)};

        MakeBuffers(pos, colour);

        _col = colour;
    }

    void Rectangle::Init() {
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

        _pixelSizeLocation = GetUniformLocation("pixel_size");

        _colLocation = GetUniformLocation("RectCol");

        if (_MVLocation == 0xffffffff || _PLocation == 0xffffffff || _colLocation == 0xffffffff
            || _minsLocation == 0xffffffff || _maxsLocation == 0xffffffff
            || _pixelSizeLocation == 0xffffffff)
            throw std::runtime_error("OpenGL: Rectangle: Failed to initialise uniform locations");

        _posBufLocation = GetAttribLocation("PosBuf");

        if (_posBufLocation == 0xffffffff)
            throw std::runtime_error("OpenGL: Rectangle: Failed to initialise buffer locations");
    }

    void Rectangle::MakeBuffers(std::vector<Vector3f> &positions, Vector4f &col) {
        _haveBuffers = false;
        _positionBuffer = std::make_shared<AttributeBuffer>(positions, static_cast<GLuint>(_posBufLocation));
        std::vector<unsigned int> els = {0, 1, 2, 2, 3, 0}; // TODO: this could be user defined...
        _indexBuffer = std::make_shared<ArrayBuffer>(els, GL_ELEMENT_ARRAY_BUFFER);

        _mins = positions[0];
        _maxs = positions[2];

        _limits << positions[0].x, positions[2].x, positions[2].y, positions[0].y, positions[2].z, positions[0].z;

        _haveBuffers = true;
    }

    void Rectangle::Render(const Matrix4f &MV, const Matrix4f &P, float pix_size) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        Enable();
        SetModelView(MV);
        SetProj(P);

        // Set the limits of the rectangle
        glFuncs->glUniform3f(_minsLocation, _mins.x, _mins.y, _mins.z);
        glFuncs->glUniform3f(_maxsLocation, _maxs.x, _maxs.y, _maxs.z);
        //
        glFuncs->glUniform1f(_pixelSizeLocation, pix_size);
        // Set the colour
        glFuncs->glUniform4f(_colLocation, _col.x, _col.y, _col.z, _col.w);

        _positionBuffer->Bind();
        _indexBuffer->Bind();

        glFuncs->glDrawElements(GL_TRIANGLES, 6, _indexBuffer->getType(), nullptr);

        _positionBuffer->Unbind();
        _indexBuffer->Unbind();

        Disable();
    }

}