//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLVERTEXBUFFER_H
#define CLTEM_OGLVERTEXBUFFER_H


#include <QtOpenGL>
#include <QOpenGLFunctions>

#include "oglmaths.h"

class OGLVertexBuffer
{
public:
    OGLVertexBuffer(std::vector<Vector3f> BufferData, GLuint BufferIndex);
    OGLVertexBuffer(std::vector<Vector4f> BufferData, GLuint BufferIndex);

    void Draw(bool draw, GLenum draw_type = GL_LINES);

    void Delete();

private:

    GLuint _BufferPtr, _BufferIndex;

    unsigned int _Size, _SizePer;

    bool Exists();
};


#endif //CLTEM_OGLVERTEXBUFFER_H
