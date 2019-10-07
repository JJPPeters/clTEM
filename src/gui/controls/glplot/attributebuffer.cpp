//
// Created by jon on 10/09/18.
//

#include "attributebuffer.h"

namespace PGL {

    void AttributeBuffer::Bind() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glBindBuffer(_buffer_target, _buffer_ptr);
        glFuncs->glEnableVertexAttribArray(_buffer_location);

        glFuncs->glVertexAttribPointer(_buffer_location, _size_per, _data_type, GL_FALSE, 4 * _stride, (GLvoid*)(4 * _offset));
    }

    void AttributeBuffer::Unbind() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glBindBuffer(_buffer_target, 0);
        glFuncs->glDisableVertexAttribArray(_buffer_location);
    }
}