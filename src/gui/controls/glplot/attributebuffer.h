//
// Created by jon on 10/09/18.
//

#ifndef CLTEM_ATTRIBUTEBUFFER_H
#define CLTEM_ATTRIBUTEBUFFER_H

#include "arraybuffer.h"

namespace PGL {

    class AttributeBuffer : public ArrayBuffer {
    public:
        AttributeBuffer(std::vector<int> BufferData, GLuint BufferLocation, GLenum BufferType = GL_ARRAY_BUFFER)
                : ArrayBuffer(BufferData, BufferType) {
            _data_type = GL_INT;
            _buffer_location = BufferLocation;
            _stride = 0;
            _offset = 0;
        }

        AttributeBuffer(std::vector<float> BufferData, GLuint BufferLocation, GLenum BufferType = GL_ARRAY_BUFFER)
                : ArrayBuffer(BufferData, BufferType) {
            _data_type = GL_FLOAT;
            _buffer_location = BufferLocation;
            _stride = 0;
            _offset = 0;
        }

        AttributeBuffer(std::vector<Vector3f> BufferData, GLuint BufferLocation, GLenum BufferType = GL_ARRAY_BUFFER)
                : ArrayBuffer(BufferData, BufferType) {
            _data_type = GL_FLOAT;
            _buffer_location = BufferLocation;
            _stride = 0;
            _offset = 0;
        }

        void Bind();

        void Unbind();

    private:
        GLuint _buffer_location, _stride, _offset;

    };

}

#endif //CLTEM_ATTRIBUTEBUFFER_H
