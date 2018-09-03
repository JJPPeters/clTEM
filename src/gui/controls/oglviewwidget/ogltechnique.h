//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLTECHNIQUE_H
#define CLTEM_OGLTECHNIQUE_H


#include <QtOpenGL>

class OGLTechnique
{
public:
    OGLTechnique();

    virtual ~OGLTechnique();

    virtual bool Init();

    void Enable();

    void Disable();

protected:
    bool AddShader(GLenum ShaderType, std::string shdr);

    bool Finalise();

    GLuint GetUniformLocation(const char* pUniformName);

    GLint GetProgramParam(GLint param);

    GLuint _shaderProg;

private:
    typedef std::list<GLuint> ShaderObjectList;
    ShaderObjectList _shaderObjectList;
};


#endif //CLTEM_OGLTECHNIQUE_H
