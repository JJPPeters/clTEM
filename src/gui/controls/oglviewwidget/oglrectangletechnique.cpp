//
// Created by Jon on 03/09/2018.
//

#include <iostream>
#include "oglrectangletechnique.h"

OGLRectangleTechnique::OGLRectangleTechnique()
{
    _haveBuffers = false;
    Q_INIT_RESOURCE(shaders);
}

bool OGLRectangleTechnique::Init() {
    if (!OGLTechnique::Init())
        throw std::runtime_error("OpenGL: Rectangle: Failed to initialise technique base");

    QFile f_vert(":/OGL/Shaders/rectangle.vs");
    if (!f_vert.open(QFile::ReadOnly | QFile::Text)) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to read vertex shader");
    }
    auto s_vert = QTextStream(&f_vert).readAll().toStdString();
    if (!CompileShader(GL_VERTEX_SHADER, s_vert)) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to initialise vertex shader");
    }
    f_vert.close();

    QFile f_frag(":/OGL/Shaders/rectangle.fs");
    if (!f_frag.open(QFile::ReadOnly | QFile::Text)) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to read fragment shader");
    }
    auto s_frag = QTextStream(&f_frag).readAll().toStdString();
    if (!CompileShader(GL_FRAGMENT_SHADER, s_frag)) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to initialise fragment shader");
    }
    f_frag.close();

    if (!Finalise()) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to finalise shaders");
    }

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

    return true;
}

void OGLRectangleTechnique::MakeBuffers(std::vector<Vector3f> &positions, Vector4f &col, Vector3f &mins, Vector3f &maxs)
{
    _haveBuffers = false;
    _positionBuffer = std::make_shared<OGLAttributeBuffer>(OGLAttributeBuffer(positions, static_cast<GLuint>(_posBufLocation)));
    std::vector<GLuint> els = {0, 1, 2, 2, 3, 0}; // TODO: this could be user defined...
    _indexBuffer = std::make_shared<OGLArrayBuffer>(OGLArrayBuffer(els, GL_ELEMENT_ARRAY_BUFFER));

    // TODO: this should be set when the square is defined
    _mins = mins;
    _maxs = maxs;
    _col = col;

    _haveBuffers = true;
}

bool OGLRectangleTechnique::Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize)
{
    if (!_haveBuffers)
        return false;

    Enable();
    SetModelView(MV);
    SetProj(P);
    SetLims(_mins, _maxs);
    SetCol(_col);

    _positionBuffer->DrawArrays(false);
    _indexBuffer->DrawElements(true, GL_TRIANGLES);

    Disable();

    return true;
}

void OGLRectangleTechnique::SetLims(const Vector3f &mins, const Vector3f & maxs) {
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glUniform3f(_minsLocation, mins.x, mins.y, mins.z);
    glFuncs->glUniform3f(_maxsLocation, maxs.x, maxs.y, maxs.z);
}

void OGLRectangleTechnique::SetCol(const Vector4f &col) {
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glUniform4f(_colLocation, col.x, col.y, col.z, col.w);
}