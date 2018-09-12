//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLTECHNIQUE_H
#define CLTEM_OGLTECHNIQUE_H


#include <QtOpenGL>

#include "oglmaths.h"
class OGLTechnique
{
public:
    OGLTechnique();

    virtual ~OGLTechnique();

    virtual bool Init();

    void Enable();

    void Disable();

protected:
    bool CompileShader(GLenum ShaderType, std::string shdr);

    bool Finalise();

    GLuint GetUniformLocation(const char* pUniformName);

    GLint GetProgramParam(GLint param);

    GLint GetAttribLocation(std::string name);

    void SetProj(const Matrix4f& P);
    void SetModelView(const Matrix4f& MV);

    GLuint _shaderProg;

    GLuint _PLocation;
    GLuint _MVLocation;

private:
    typedef std::list<GLuint> ShaderObjectList;
    ShaderObjectList _shaderObjectList;
};


#endif //CLTEM_OGLTECHNIQUE_H
