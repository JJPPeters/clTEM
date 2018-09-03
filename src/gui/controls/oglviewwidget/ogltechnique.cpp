//
// Created by Jon on 03/09/2018.
//

#include "ogltechnique.h"
#include "oglutils.h"

#include <iostream>

OGLTechnique::OGLTechnique()
{
    _shaderProg = 0;
}

OGLTechnique::~OGLTechnique()
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    for (ShaderObjectList::iterator it = _shaderObjectList.begin(); it != _shaderObjectList.end(); ++it)
        glFuncs->glDeleteShader(*it);

    if (_shaderProg != 0)
    {
        glFuncs->glDeleteProgram(_shaderProg);
        _shaderProg = 0;
    }
}

bool OGLTechnique::Init()
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    _shaderProg = glFuncs->glCreateProgram();

    if (_shaderProg == 0)
        return false;
    return true;
}

bool OGLTechnique::AddShader(GLenum ShaderType, const char *pFileName)
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    std::string s;

    if (!ReadFile(pFileName, s))
        return false;

    GLuint ShaderObject = glFuncs->glCreateShader(ShaderType);

    if (ShaderObject == 0)
        return false;

    _shaderObjectList.push_back(ShaderObject);

    const GLchar* p[1];
    p[0] = s.c_str();

    GLint Lengths[1] = { (GLint)s.size() };

    glFuncs->glShaderSource(ShaderObject, 1, p, Lengths);

    glFuncs->glCompileShader(ShaderObject);

    GLint success;
    glFuncs->glGetShaderiv(ShaderObject, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        GLchar InfoLog[1024];
        glFuncs->glGetShaderInfoLog(ShaderObject, 1024, NULL, InfoLog);
        std::cout << InfoLog << std::endl;
        return false;
    }

    glFuncs->glAttachShader(_shaderProg, ShaderObject);

    return true;
}

bool OGLTechnique::Finalise()
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glFuncs->glLinkProgram(_shaderProg);

    glFuncs->glGetProgramiv(_shaderProg, GL_LINK_STATUS, &Success);

    if (Success == 0)
    {
        glFuncs->glGetProgramInfoLog(_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        std::cout << ErrorLog << std::endl;
        return false;
    }

    glFuncs->glValidateProgram(_shaderProg);
    glFuncs->glGetProgramiv(_shaderProg, GL_VALIDATE_STATUS, &Success);
    if (!Success)
    {
        glFuncs->glGetProgramInfoLog(_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        std::cout << ErrorLog << std::endl;
        //   return false;
    }

    // Delete the intermediate shader objects that have been added to the program
    for (ShaderObjectList::iterator it = _shaderObjectList.begin(); it != _shaderObjectList.end(); ++it)
    {
        glFuncs->glDeleteShader(*it);
    }

    _shaderObjectList.clear();

    return OGLCheckError();
}

void OGLTechnique::Enable()
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glUseProgram(_shaderProg);
}

void OGLTechnique::Disable()
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glUseProgram(0);
}

GLuint OGLTechnique::GetUniformLocation(const char *pUniformName)
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    GLuint Location = glFuncs->glGetUniformLocation(_shaderProg, pUniformName);

    if (Location == 0xffffffff) {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }

    return Location;
}

GLint OGLTechnique::GetProgramParam(GLint param)
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    GLint ret;
    glFuncs->glGetProgramiv(_shaderProg, param, &ret);
    return ret;
}
