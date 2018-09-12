//
// Created by jon on 10/09/18.
//

#include "oglattributebuffer.h"

//OGLAttributeBuffer::OGLAttributeBuffer(std::vector<Vector3f> BufferData, GLuint BufferIndex, GLenum BufferType) : OGLArrayBuffer(BufferData, BufferType) {
//    _BufferIndex = BufferIndex;
//}
//
//OGLAttributeBuffer::OGLAttributeBuffer(std::vector<Vector4f> BufferData, GLuint BufferIndex, GLenum BufferType) : OGLArrayBuffer(BufferData, BufferType) {
//    _BufferIndex = BufferIndex;
//}

void OGLAttributeBuffer::DrawArrays(bool doDraw, GLenum draw_type) {
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glEnableVertexAttribArray(_BufferIndex);

    glFuncs->glBindBuffer(_BufferType, _BufferPtr);

    glFuncs->glVertexAttribPointer(_BufferIndex, _SizePer, GL_FLOAT, GL_FALSE, 0, nullptr);
    if (doDraw)
        glDrawArrays(draw_type, 0, _Size);

    glFuncs->glBindBuffer(_BufferType, 0); // "unbind"

    // interestingly, using disable here cocks things up. I must not understand it properly.
    //glFuncs->glDisableVertexAttribArray(_BufferIndex);
    glFuncs->glEnableVertexAttribArray(0);
}

void OGLAttributeBuffer::DrawElements(bool doDraw, GLenum draw_type) {
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glEnableVertexAttribArray(_BufferIndex);

    glFuncs->glBindBuffer(_BufferType, _BufferPtr);

    glFuncs->glVertexAttribPointer(_BufferIndex, _SizePer, GL_FLOAT, GL_FALSE, 0, nullptr);
    if (doDraw)
        glDrawElements(draw_type, _Size, GL_UNSIGNED_INT, nullptr);

    glFuncs->glBindBuffer(_BufferType, 0); // "unbind"

    // interestingly, using disable here cocks things up. I must not understand it properly.
    //glFuncs->glDisableVertexAttribArray(_BufferIndex);
    glFuncs->glEnableVertexAttribArray(0);
}
