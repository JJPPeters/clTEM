//
// Created by Jon on 14/10/2019.
//

#include "shader.h"

namespace PGL {
    Shader::Shader() {
        _shaderProg = 0;
        _PLocation = 0;
        _MVLocation = 0;

        AutoShaderResource::GetInstance();
    }

    Shader::~Shader() {
        auto con = QOpenGLContext::currentContext();
        if (!con)
            return;

        QOpenGLFunctions *glFuncs = con->functions();
        glFuncs->initializeOpenGLFunctions();

        if (_shaderProg != 0) {
            glFuncs->glDeleteProgram(_shaderProg);
            _shaderProg = 0;
        }
    }

    void Shader::initialise() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        _shaderProg = glFuncs->glCreateProgram();

        if (_shaderProg == 0)
            throw std::runtime_error("Error initialising technique");
    }

    void Shader::finalise() {
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

        for (auto &shader: _shaderObjectList) {
            glFuncs->glDetachShader(_shaderProg, shader); // not really needed
            glFuncs->glDeleteShader(shader);
        }

        _shaderObjectList.clear();

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

    void Shader::enable() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glUseProgram(_shaderProg);
    }

    void Shader::disable() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glUseProgram(0);
    }

    void Shader::compileFromString(GLenum shader_type, std::string shader) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        GLuint _shaderObj = glFuncs->glCreateShader(shader_type);

        if (_shaderObj == 0)
            throw std::runtime_error("Error creating shader object");

//        const GLchar *p[1] = {shader.c_str()};
        std::vector<const GLchar *> p = {shader.c_str()};
        auto Lengths = (GLint) shader.size();

        glFuncs->glShaderSource(_shaderObj, 1, &p[0], &Lengths);

        glFuncs->glCompileShader(_shaderObj);

        GLint success;
        glFuncs->glGetShaderiv(_shaderObj, GL_COMPILE_STATUS, &success);

        if (success == GL_FALSE) {
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

    void Shader::compileFromFile(GLenum shader_type, std::string file_path) {
        QFile f_vert(QString::fromStdString(file_path));
        if (!f_vert.open(QFile::ReadOnly | QFile::Text))
            throw std::runtime_error("Error, failed to open shader file: " + file_path);

        auto s_vert = QTextStream(&f_vert).readAll().toStdString();
        compileFromString(shader_type, s_vert);
        f_vert.close();
    }

    GLuint Shader::getUniformLocation(std::string name) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        GLuint Location = glFuncs->glGetUniformLocation(_shaderProg, name.c_str());

        if (Location == 0xffffffff)
            throw std::runtime_error("Error getting the location of uniform: " + name);

        return Location;
    }

    GLuint Shader::getAttribLocation(std::string name) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        return glFuncs->glGetAttribLocation(_shaderProg, name.c_str());
    }

    void Shader::setModelView(const Matrix4f& MV)
    {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glUniformMatrix4fv(_MVLocation, 1, GL_TRUE, (const GLfloat*)MV.m);
    }

    void Shader::setProj(const Matrix4f& P)
    {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glUniformMatrix4fv(_PLocation, 1, GL_TRUE, (const GLfloat*)P.m);
    }
}