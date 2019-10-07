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

        template<typename T>
        ArrayBuffer(std::vector<T> BufferData, GLenum data_type, GLenum buffer_target = GL_ARRAY_BUFFER) {
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

        void Delete();

        unsigned int getSize() { return _size; }

    protected:

        GLuint _buffer_ptr;

        GLenum _buffer_target;

        unsigned int _size, _size_per;

        bool Exists();

    };

}

#endif //CLTEM_OGLVERTEXBUFFER_H
