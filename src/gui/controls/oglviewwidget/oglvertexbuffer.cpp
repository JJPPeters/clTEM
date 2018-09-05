//
// Created by Jon on 03/09/2018.
//

#include "oglvertexbuffer.h"

OGLVertexBuffer::OGLVertexBuffer(std::vector<Vector3f> BufferData, GLuint BufferIndex) {
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    _BufferIndex = BufferIndex;

    _BufferPtr = 0xffffffff;
    _Size = static_cast<unsigned int>(BufferData.size());
    _SizePer = 3;

    glFuncs->glGenBuffers(1, &_BufferPtr);
    glFuncs->glBindBuffer(GL_ARRAY_BUFFER, _BufferPtr);
    glFuncs->glBufferData(GL_ARRAY_BUFFER, _Size * sizeof(Vector3f), &BufferData[0], GL_DYNAMIC_DRAW); // TODO: difference to static/dynamic draw?

    if (_BufferPtr == 0xffffffff)
        throw std::runtime_error("OpenGL: Failed to initialise vertex buffers.");
}

// TODO: make this better with templates etc...
OGLVertexBuffer::OGLVertexBuffer(std::vector<Vector4f> BufferData, GLuint BufferIndex) {
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    _BufferIndex = BufferIndex;

    _BufferPtr = 0xffffffff;
    _Size = static_cast<unsigned int>(BufferData.size());
    _SizePer = 4;

    glFuncs->glGenBuffers(1, &_BufferPtr);
    glFuncs->glBindBuffer(GL_ARRAY_BUFFER, _BufferPtr);
    glFuncs->glBufferData(GL_ARRAY_BUFFER, _Size * sizeof(Vector4f), &BufferData[0], GL_DYNAMIC_DRAW); // TODO: difference to static/dynamic draw?

    if (_BufferPtr == 0xffffffff)
        throw std::runtime_error("OpenGL: Failed to initialise vertex buffers.");
}

void OGLVertexBuffer::Delete()
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();
    if (Exists())
        glFuncs->glDeleteBuffers(1, &_BufferPtr);
}

bool OGLVertexBuffer::Exists()
{
    return _BufferPtr != 0xffffffff;
}

void OGLVertexBuffer::Draw(bool doDraw, GLenum draw_type)
{
    if (!Exists())
        return;

    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glEnableVertexAttribArray(_BufferIndex);

    glFuncs->glBindBuffer(GL_ARRAY_BUFFER, _BufferPtr);

    glFuncs->glVertexAttribPointer(_BufferIndex, _SizePer, GL_FLOAT, GL_FALSE, 0, nullptr);
    if (doDraw)
        glDrawArrays(draw_type, 0, _Size);

    glFuncs->glBindBuffer(GL_ARRAY_BUFFER, 0); // "unbind"

    // interestingly, using disable here cocks things up. I must not understand it properly.
    //glFuncs->glDisableVertexAttribArray(_BufferIndex);
    glFuncs->glEnableVertexAttribArray(0);
}

