//
// Created by Jon on 03/09/2018.
//

#include <iostream>
#include "oglbillboardtechnique.h"

OGLBillBoardTechnique::OGLBillBoardTechnique()
{
    _haveBuffers = false;
    Q_INIT_RESOURCE(shaders);
}

bool OGLBillBoardTechnique::Init()
{
    if (!OGLTechnique::Init()) {
        throw std::runtime_error("OpenGL: Failed to initialise technique base");
    }

    auto v = glGetError();

    QFile f_vert(":/OGL/Shaders/billboard.vs");
    if (!f_vert.open(QFile::ReadOnly | QFile::Text)) {
        throw std::runtime_error("OpenGL: Billboard: Failed to read vertex shader");
    }
    auto s_vert = QTextStream(&f_vert).readAll().toStdString();
    if (!CompileShader(GL_VERTEX_SHADER, s_vert)) {
        throw std::runtime_error("OpenGL: Billboard: Failed to initialise vertex shader");
    }
    f_vert.close();

    QFile f_geo(":/OGL/Shaders/billboard.gs");
    if (!f_geo.open(QFile::ReadOnly | QFile::Text)) {
        throw std::runtime_error("OpenGL: Billboard: Failed to read vertex shader");
    }
    auto s_geo = QTextStream(&f_geo).readAll().toStdString();
    if (!CompileShader(GL_GEOMETRY_SHADER, s_geo)) {
        throw std::runtime_error("OpenGL: Billboard: Failed to initialise geometry shader");
    }
    f_geo.close();

    auto v3 = glGetError();

    QFile f_frag(":/OGL/Shaders/billboard.fs");
    if (!f_frag.open(QFile::ReadOnly | QFile::Text)) {
        throw std::runtime_error("OpenGL: Billboard: Failed to read fragment shader");
    }
    auto s_frag = QTextStream(&f_frag).readAll().toStdString();
    if (!CompileShader(GL_FRAGMENT_SHADER, s_frag)) {
        throw std::runtime_error("OpenGL: Billboard: Failed to initialise fragment shader");
    }
    f_frag.close();

    if (!Finalise()) {
        throw std::runtime_error("OpenGL: Billboard: Failed to finalise shaders");
    }

    _MVLocation = GetUniformLocation("ModelView");
    _PLocation = GetUniformLocation("Proj");

    if (_MVLocation == 0xffffffff || _PLocation == 0xffffffff)
        throw std::runtime_error("OpenGL: Failed to initialise uniform locations");

    _colBufLocation = GetAttribLocation("ColBuf");
    _posBufLocation = GetAttribLocation("PosBuf");

    if (_posBufLocation == 0xffffffff || _colBufLocation == 0xffffffff) {
        throw std::runtime_error("OpenGL: Failed to initialise buffer locations");
    }

    return true;
}

void OGLBillBoardTechnique::MakeBuffers(std::vector<Vector3f>& positions, std::vector<Vector3f>& colours)
{
    _haveBuffers = false;
    _colourBuffer = std::make_shared<OGLAttributeBuffer>(OGLAttributeBuffer(colours, static_cast<GLuint>(_colBufLocation)));
    _positionBuffer = std::make_shared<OGLAttributeBuffer>(OGLAttributeBuffer(positions, static_cast<GLuint>(_posBufLocation)));
    _haveBuffers = true;
}

bool OGLBillBoardTechnique::Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize)
{
    if (!_haveBuffers)
        return false;

    Enable();
    SetProj(P);
    SetModelView(MV);

    _colourBuffer->DrawArrays(false);
    _positionBuffer->DrawArrays(true, GL_POINTS);

    Disable();

    return true;
}