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

bool OGLRectangleTechnique::Init()
{
    if (!OGLTechnique::Init()) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to initialise technique base");
    }

    QFile f_vert(":/OGL/Shaders/rectangle.vs");
    if (!f_vert.open(QFile::ReadOnly | QFile::Text)) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to read vertex shader");
    }
    auto s_vert = QTextStream(&f_vert).readAll().toStdString();
    if (!AddShader(GL_VERTEX_SHADER, s_vert)) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to initialise vertex shader");
    }
    f_vert.close();

    QFile f_frag(":/OGL/Shaders/rectangle.fs");
    if (!f_frag.open(QFile::ReadOnly | QFile::Text)) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to read fragment shader");
    }
    auto s_frag = QTextStream(&f_frag).readAll().toStdString();
    if (!AddShader(GL_FRAGMENT_SHADER, s_frag)) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to initialise fragment shader");
    }
    f_frag.close();

    if (!Finalise()) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to finalise shaders");
    }

    _MVLocation = GetUniformLocation("ModelView");
    _PLocation = GetUniformLocation("Proj");

    if (_MVLocation == 0xffffffff || _PLocation == 0xffffffff)
        throw std::runtime_error("OpenGL: Rectangle: Failed to initialise uniform locations");

    _colBufLocation = GetAttribLocation("ColBuf");
    _posBufLocation = GetAttribLocation("PosBuf");

    if (_posBufLocation == 0xffffffff || _colBufLocation == 0xffffffff) {
        throw std::runtime_error("OpenGL: Rectangle: Failed to initialise buffer locations");
    }

    return true;
}

void OGLRectangleTechnique::MakeBuffers(std::vector<Vector3f>& positions, std::vector<Vector4f>& colours)
{
    _haveBuffers = false;
    _colourBuffer = std::make_shared<OGLVertexBuffer>(OGLVertexBuffer(colours, static_cast<GLuint>(_colBufLocation)));
    _positionBuffer = std::make_shared<OGLVertexBuffer>(OGLVertexBuffer(positions, static_cast<GLuint>(_posBufLocation)));
    _haveBuffers = true;
}

bool OGLRectangleTechnique::Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize)
{
    if (!_haveBuffers)
        return false;

    Enable();
    SetModelView(MV);
    SetProj(P);

    _colourBuffer->Draw(false);
    _positionBuffer->Draw(true, GL_TRIANGLES);

    Disable();

    return true;
}