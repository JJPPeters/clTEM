//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLVERTEXBUFFER_H
#define CLTEM_OGLVERTEXBUFFER_H


#include <QtOpenGL>
#include <QOpenGLFunctions>

#include "oglmaths.h"

namespace PGL {

    class ArrayBuffer {
    public:
        // TODO: add update functions

        ArrayBuffer(std::vector<unsigned int> BufferData, GLenum buffer_target = GL_ARRAY_BUFFER) {
            _data_type = GL_UNSIGNED_INT;

            InitBuffer(BufferData, buffer_target);
        }

        ArrayBuffer(std::vector<int> BufferData, GLenum buffer_target = GL_ARRAY_BUFFER) {
            _data_type = GL_INT;

            InitBuffer(BufferData, buffer_target);
        }

        ArrayBuffer(std::vector<float> BufferData, GLenum buffer_target = GL_ARRAY_BUFFER) {
            _data_type = GL_FLOAT;

            InitBuffer(BufferData, buffer_target);
        }

        ArrayBuffer(std::vector<Vector3f> BufferData, GLenum buffer_target = GL_ARRAY_BUFFER) {
            _data_type = GL_FLOAT;

            InitBuffer(BufferData, buffer_target);
        }

        void Delete();

        void Bind();

        void Unbind();

        unsigned int getSize() { return _size; }

        GLenum getType() {return _data_type;}

    protected:

        template<typename T>
        void InitBuffer(std::vector<T> BufferData, GLenum buffer_target = GL_ARRAY_BUFFER) {
            QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
            glFuncs->initializeOpenGLFunctions();

            _buffer_ptr = 0xffffffff;
            _size = static_cast<unsigned int>(BufferData.size());
            _buffer_target = buffer_target;

            _size_per = sizeof(T) / sizeof(float);

            glFuncs->glGenBuffers(1, &_buffer_ptr);

            if (_buffer_ptr == 0xffffffff)
                throw std::runtime_error("OpenGL: Failed to initialise vertex buffers.");

            glFuncs->glBindBuffer(_buffer_target, _buffer_ptr);
            // TODO: difference to static/dynamic draw?
            glFuncs->glBufferData(_buffer_target, _size * sizeof(T), &BufferData[0], GL_STATIC_DRAW);
            glFuncs->glBindBuffer(_buffer_target, 0);
        }

        GLuint _buffer_ptr;

        GLenum _buffer_target;

        unsigned int _size, _size_per;

        GLenum _data_type;

        bool Exists();

    };

}

#endif //CLTEM_OGLVERTEXBUFFER_H
