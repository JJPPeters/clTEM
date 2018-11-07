//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLVERTEXBUFFER_H
#define CLTEM_OGLVERTEXBUFFER_H


#include <QtOpenGL>
#include <QOpenGLFunctions>

#include "oglmaths.h"

class OGLArrayBuffer
{
public:
    template <typename T>
    explicit OGLArrayBuffer(std::vector<T> BufferData, GLenum BufferType = GL_ARRAY_BUFFER) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        _BufferPtr = 0xffffffff;
        _Size = static_cast<unsigned int>(BufferData.size());
        _BufferType = BufferType;

        _SizePer = sizeof(T) / sizeof(float);

        glFuncs->glGenBuffers(1, &_BufferPtr);
        glFuncs->glBindBuffer(_BufferType, _BufferPtr);
        glFuncs->glBufferData(_BufferType, _Size * sizeof(T), &BufferData[0], GL_STATIC_DRAW); // TODO: difference to static/dynamic draw?

        if (_BufferPtr == 0xffffffff)
            throw std::runtime_error("OpenGL: Failed to initialise vertex buffers.");
    }
//    OGLArrayBuffer(std::vector<Vector4f> BufferData, GLenum BufferType = GL_ARRAY_BUFFER);

    void DrawArrays(bool doDraw = false, GLenum draw_type = GL_LINES);

    void DrawElements(bool doDraw = false, GLenum draw_type = GL_LINES);

    void Delete();

protected:

    GLuint _BufferPtr;

    GLenum _BufferType;

    unsigned int _Size, _SizePer;

    bool Exists();
};


#endif //CLTEM_OGLVERTEXBUFFER_H
