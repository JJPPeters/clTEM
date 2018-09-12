//
// Created by Jon on 03/09/2018.
//

#include "oglarraybuffer.h"

void OGLArrayBuffer::Delete()
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();
    if (Exists())
        glFuncs->glDeleteBuffers(1, &_BufferPtr);
}

bool OGLArrayBuffer::Exists()
{
    return _BufferPtr != 0xffffffff;
}



void OGLArrayBuffer::DrawArrays(bool doDraw, GLenum draw_type) {
    if (!Exists())
        return;

    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glBindBuffer(_BufferType, _BufferPtr);

    if (doDraw)
        glDrawArrays(draw_type, 0, _Size);

    glFuncs->glBindBuffer(_BufferType, 0); // "unbind"
}

void OGLArrayBuffer::DrawElements(bool doDraw, GLenum draw_type) {
    if (!Exists())
        return;

    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glBindBuffer(_BufferType, _BufferPtr);

    if (doDraw)
        glDrawElements(draw_type, _Size, GL_UNSIGNED_INT, nullptr);

    glFuncs->glBindBuffer(_BufferType, 0); // "unbind"
}

