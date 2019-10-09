//
// Created by Jon on 03/09/2018.
//

#include "arraybuffer.h"

namespace PGL {

    void ArrayBuffer::Delete() {
        auto con = QOpenGLContext::currentContext();
        if (!con)
            return;

        QOpenGLFunctions *glFuncs = con->functions();
        glFuncs->initializeOpenGLFunctions();
        if (Exists())
            glFuncs->glDeleteBuffers(1, &_buffer_ptr);
    }

    bool ArrayBuffer::Exists() {
        return _buffer_ptr != 0xffffffff;
    }

    void ArrayBuffer::Bind() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glBindBuffer(_buffer_target, _buffer_ptr);
    }

    void ArrayBuffer::Unbind() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glBindBuffer(_buffer_target, 0);
    }

}
