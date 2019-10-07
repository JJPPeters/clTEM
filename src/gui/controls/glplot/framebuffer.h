//
// Created by Jon on 07/10/2019.
//

#ifndef CLTEM_FRAMEBUFFER_H
#define CLTEM_FRAMEBUFFER_H

#include <QtOpenGL>
#include <QOpenGLFunctions>
#include "error.h"

namespace PGL {
    class Framebuffer {
    public:
        Framebuffer(int width, int height, float scaling = 1.0, unsigned int multisampling = 1);

        ~Framebuffer() {};

        void Resize(int width, int height, float scaling);

        void Bind();

        void Unbind();

        void Blit(GLuint destination_buffer);

    private:
        int _width, _height;
        unsigned int _multisampling;
        float _scaling;

        GLuint _fbo, _rbo_c, _rbo_d;

    };
}


#endif //CLTEM_FRAMEBUFFER_H
