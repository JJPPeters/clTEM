//
// Created by jon on 10/09/18.
//

#ifndef CLTEM_OGLATTRIBUTEBUFFER_H
#define CLTEM_OGLATTRIBUTEBUFFER_H

#include "oglarraybuffer.h"

class OGLAttributeBuffer : public OGLArrayBuffer {
public:
    template <typename T>
    OGLAttributeBuffer(std::vector<T> BufferData, GLuint BufferIndex, GLenum BufferType = GL_ARRAY_BUFFER): OGLArrayBuffer(BufferData, BufferType) {
        _BufferIndex = BufferIndex;
    }

//    OGLAttributeBuffer(std::vector<Vector4f> BufferData, GLuint BufferIndex, GLenum BufferType = GL_ARRAY_BUFFER);

    void DrawArrays(bool doDraw = false, GLenum draw_type = GL_LINES);

    void DrawElements(bool doDraw = false, GLenum draw_type = GL_LINES);

private:
    GLuint _BufferIndex;
};


#endif //CLTEM_OGLATTRIBUTEBUFFER_H
