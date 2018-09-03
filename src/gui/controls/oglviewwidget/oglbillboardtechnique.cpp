//
// Created by Jon on 03/09/2018.
//

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

    QFile f_vert(":/OGL/Shaders/billboard.vs");
    if (!f_vert.open(QFile::ReadOnly | QFile::Text)) {
        throw std::runtime_error("OpenGL: Failed to read vertex shader");
    }
    auto s_vert = QTextStream(&f_vert).readAll().toStdString();
    if (!AddShader(GL_VERTEX_SHADER, s_vert)) {
        throw std::runtime_error("OpenGL: Failed to initialise vertex shader");
    }
    f_vert.close();

    QFile f_frag(":/OGL/Shaders/billboard.fs");
    if (!f_frag.open(QFile::ReadOnly | QFile::Text)) {
        throw std::runtime_error("OpenGL: Failed to read fragment shader");
    }
    auto s_frag = QTextStream(&f_frag).readAll().toStdString();
    if (!AddShader(GL_FRAGMENT_SHADER, s_frag)) {
        throw std::runtime_error("OpenGL: Failed to initialise fragment shader");
    }

    if (!Finalise()) {
        throw std::runtime_error("OpenGL: Failed to finalise shaders");
    }

    _MVLocation = GetUniformLocation("ModelView");
    _PLocation = GetUniformLocation("Proj");
    _TextureLocation = GetUniformLocation("SphereTexture");
    _ScreenSizeLocation = GetUniformLocation("ScreenSize");

    if (_MVLocation == 0xffffffff ||
        _PLocation == 0xffffffff ||
        _TextureLocation == 0xffffffff ||
        _ScreenSizeLocation == 0xffffffff)
    {
        throw std::runtime_error("OpenGL: Failed to initialise uniform locations");
    }

    _sphereTexture = std::make_shared<OGLTexture>(OGLTexture(GL_TEXTURE_2D));

    QFile f_tex(":/OGL/Shaders/sphere.bin");
    if (!f_tex.open(QFile::ReadOnly)) {
        throw std::runtime_error("OpenGL: Failed to read sphere texture");
    }
    auto b_tex = f_tex.readAll();
    std::vector<unsigned char> c_tex(b_tex.begin(), b_tex.end());

    int test = b_tex.size();
    int teste = c_tex.size();

    if (!_sphereTexture->LoadCharArrayFromFile(c_tex, 64, 64))
    {
        throw std::runtime_error("OpenGL: Failed to load sphere texture");
        return false;
    }

    _colBufLocation = GetAttribLocation("ColBuf");
    _posBufLocation = GetAttribLocation("PosBuf");

    if (_posBufLocation == 0xffffffff ||
        _colBufLocation == 0xffffffff) {
        throw std::runtime_error("OpenGL: Failed to initialise buffer locations");
    }

    return true;
}

void OGLBillBoardTechnique::MakeBuffers(std::vector<Vector3f>& positions, std::vector<Vector3f>& colours)
{
    _haveBuffers = false;

    _colourBuffer = std::make_shared<OGLVertexBuffer>(OGLVertexBuffer(colours, _colBufLocation));

    _positionBuffer = std::make_shared<OGLVertexBuffer>(OGLVertexBuffer(positions, _posBufLocation));

    _haveBuffers = true;
}

void OGLBillBoardTechnique::SetModelView(const Matrix4f& MV)
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glUniformMatrix4fv(_MVLocation, 1, GL_TRUE, (const GLfloat*)MV.m);
}

void OGLBillBoardTechnique::SetProj(const Matrix4f& P)
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glUniformMatrix4fv(_PLocation, 1, GL_TRUE, (const GLfloat*)P.m);
}


void OGLBillBoardTechnique::SetColourTextureUnit(unsigned int TextureUnit)
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glUniform1i(_TextureLocation, TextureUnit);
}

void OGLBillBoardTechnique::SetScreenSize(Vector2f size)
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glUniform2f(_ScreenSizeLocation, size.x, size.y);
}

GLint OGLBillBoardTechnique::GetAttribLocation(std::string name)
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    return glFuncs->glGetAttribLocation(_shaderProg, name.c_str());
}

bool OGLBillBoardTechnique::Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize)
{
    if (!_haveBuffers)
        return false;

    Enable();
    SetProj(P);
    SetModelView(MV);
    SetScreenSize(ScreenSize);

    _sphereTexture->Bind(GL_TEXTURE0);

    _colourBuffer->Draw(false);
    _positionBuffer->Draw(true);

    Disable();

    return true;
}