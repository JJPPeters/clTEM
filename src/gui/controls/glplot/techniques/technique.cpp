//
// Created by Jon on 03/09/2018.
//

#include "technique.h"

#include <iostream>


PGL::Technique::Technique(bool visible) {
    _shaderProg = 0;

    _limits << std::numeric_limits<float>::max(), std::numeric_limits<float>::min(),
            std::numeric_limits<float>::max(), std::numeric_limits<float>::min(),
            std::numeric_limits<float>::max(), std::numeric_limits<float>::min();

    AutoShaderResource::GetInstance();

    _visible = visible;
}

PGL::Technique::~Technique() {
    auto con = QOpenGLContext::currentContext();
    if (!con)
        return;

    QOpenGLFunctions *glFuncs = con->functions();
    glFuncs->initializeOpenGLFunctions();

    for (unsigned int &sh : _shaderObjectList)
        glFuncs->glDeleteShader(sh);

    _shaderObjectList.clear();

    if (_shaderProg != 0) {
        glFuncs->glDeleteProgram(_shaderProg);
        _shaderProg = 0;
    }
}

namespace PGL {
    void Technique::Init() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        _shaderProg = glFuncs->glCreateProgram();

        if (_shaderProg == 0)
            throw std::runtime_error("Error initialising technique");
    }

    void Technique::Finalise() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        GLint Success = 0;

        glFuncs->glLinkProgram(_shaderProg);

        glFuncs->glGetProgramiv(_shaderProg, GL_LINK_STATUS, &Success);

        if (Success == GL_FALSE) {
            int log_length = 0;
            glFuncs->glGetProgramiv(_shaderProg, GL_INFO_LOG_LENGTH, &log_length);

            std::vector<GLchar> error_log(log_length, 0);
            glFuncs->glGetProgramInfoLog(_shaderProg, log_length, nullptr, &error_log[0]);

            std::string error_log_string(error_log.begin(), error_log.end());
            throw std::runtime_error("Error finalising technique: " + error_log_string);
        }

        glFuncs->glValidateProgram(_shaderProg);
        glFuncs->glGetProgramiv(_shaderProg, GL_VALIDATE_STATUS, &Success);

        if (Success == GL_FALSE) {
            int log_length = 0;
            glFuncs->glGetProgramiv(_shaderProg, GL_INFO_LOG_LENGTH, &log_length);

            std::vector<GLchar> error_log(log_length, 0);
            glFuncs->glGetProgramInfoLog(_shaderProg, log_length, nullptr, &error_log[0]);

            std::string error_log_string(error_log.begin(), error_log.end());
            throw std::runtime_error("Error finalising technique: " + error_log_string);
        }
    }

    void Technique::Enable() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glUseProgram(_shaderProg);
    }

    void Technique::Disable() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glUseProgram(0);
    }

    GLuint Technique::GetUniformLocation(std::string name) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        GLuint Location = glFuncs->glGetUniformLocation(_shaderProg, name.c_str());

        if (Location == 0xffffffff) {
            throw std::runtime_error("Error getting the location of uniform: " + name);
        }

        return Location;
    }

    GLint Technique::GetAttribLocation(std::string name) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        return glFuncs->glGetAttribLocation(_shaderProg, name.c_str());
    }

    void Technique::SetModelView(const Matrix4f& MV)
    {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glUniformMatrix4fv(_MVLocation, 1, GL_TRUE, (const GLfloat*)MV.m);
    }

    void Technique::SetProj(const Matrix4f& P)
    {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glUniformMatrix4fv(_PLocation, 1, GL_TRUE, (const GLfloat*)P.m);
    }

    void Technique::CompileShaderFromString(GLenum ShaderType, std::string shader) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        auto _shaderObj = glFuncs->glCreateShader(ShaderType);

        if (_shaderObj == 0)
            throw std::runtime_error("Error creating shader object");

        const GLchar *p[1] = {shader.c_str()};
        GLint Lengths[1] = {(GLint) shader.size()};

        glFuncs->glShaderSource(_shaderObj, 1, p, Lengths);

        glFuncs->glCompileShader(_shaderObj);

        GLint success;
        glFuncs->glGetShaderiv(_shaderObj, GL_COMPILE_STATUS, &success);

        if (!success) {
            int log_length = 0;
            glFuncs->glGetShaderiv(_shaderObj, GL_INFO_LOG_LENGTH, &log_length);

            std::vector<GLchar> error_log(log_length, 0);
            glFuncs->glGetShaderInfoLog(_shaderObj, log_length, nullptr, &error_log[0]);

            std::string error_log_string(error_log.begin(), error_log.end());
            throw std::runtime_error("Error compiling shader: " + error_log_string);
        }

        glFuncs->glAttachShader(_shaderProg, _shaderObj);

        _shaderObjectList.push_back(_shaderObj);
    }

    void Technique::CompileShaderFromFile(GLenum ShaderType, std::string filepath) {
        QFile f_vert(QString::fromStdString(filepath));
        if (!f_vert.open(QFile::ReadOnly | QFile::Text))
            throw std::runtime_error("Error, failed to open shader file: " + filepath);

        auto s_vert = QTextStream(&f_vert).readAll().toStdString();
        CompileShaderFromString(ShaderType, s_vert);
        f_vert.close();
    }
}
