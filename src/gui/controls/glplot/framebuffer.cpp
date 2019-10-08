//
// Created by Jon on 07/10/2019.
//

#include "framebuffer.h"
namespace PGL {
    Framebuffer::Framebuffer(int width, int height, float scaling, unsigned int multisampling) {
        // Other variables are set in the Resize method
        _multisampling = multisampling;

        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glGenFramebuffers(1, &_fbo);
        glFuncs->glGenRenderbuffers(1, &_rbo_c);
        glFuncs->glGenRenderbuffers(1, &_rbo_d);

        Resize(width, height, scaling);
    }

    void Framebuffer::Resize(int width, int height, float scaling) {
        _width = static_cast<int>(width * scaling);
        _height = static_cast<int>(height * scaling);
        _scaling = scaling;

        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        QOpenGLExtraFunctions *glFuncsExtra = QOpenGLContext::currentContext()->extraFunctions();
        glFuncsExtra->initializeOpenGLFunctions();

        Bind();

        glFuncs->glBindRenderbuffer(GL_RENDERBUFFER, _rbo_c);
        if (_multisampling > 1)
            glFuncsExtra->glRenderbufferStorageMultisample(GL_RENDERBUFFER, _multisampling, GL_RGB, _width, _height);
        else
            glFuncs->glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, _width, _height);
        glFuncs->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _rbo_c);

        glFuncs->glBindRenderbuffer(GL_RENDERBUFFER, _rbo_d);
        if (_multisampling > 1)
            glFuncsExtra->glRenderbufferStorageMultisample(GL_RENDERBUFFER, _multisampling, GL_DEPTH24_STENCIL8, _width, _height);
        else
            glFuncs->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _width, _height);
        glFuncs->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo_d);

        glFuncs->glBindRenderbuffer(GL_RENDERBUFFER, 0);

        GLenum status = glFuncs->glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Error setting framebuffer size: " + PGL::ErrorToString(status));

        Unbind();
    }

    void Framebuffer::Bind() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    }

    void Framebuffer::Unbind() {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::Blit(GLuint destination_buffer) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        QOpenGLExtraFunctions *glFuncsExtra = QOpenGLContext::currentContext()->extraFunctions();
        glFuncsExtra->initializeOpenGLFunctions();

        glFuncs->glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
        glFuncs->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination_buffer);

        glFuncsExtra->glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height,
                                        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

        glFuncs->glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glFuncs->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
}