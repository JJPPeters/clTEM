//
// Created by Jon on 03/09/2018.
//

#include <QtCore>
#include "error.h"

namespace PGL {
    std::string ErrorToString(GLenum err) {
        switch (err) {
            case GL_INVALID_ENUM:
                return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE:
                return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION:
                return "GL_INVALID_OPERATION";
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return "GL_INVALID_FRAMEBUFFER_OPERATION";
            case GL_OUT_OF_MEMORY:
                return "GL_OUT_OF_MEMORY";
            case GL_STACK_UNDERFLOW:
                return "GL_STACK_UNDERFLOW";
            case GL_STACK_OVERFLOW:
                return "GL_STACK_OVERFLOW";
            case GL_FRAMEBUFFER_UNDEFINED:
                return "GL_FRAMEBUFFER_UNDEFINED";
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            case GL_FRAMEBUFFER_UNSUPPORTED:
                return "GL_FRAMEBUFFER_UNSUPPORTED";
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
            default:
                return "GL_NO_ERROR";
        }
    }

    void CheckErrors(std::string prefix) {
        GLenum err;
        std::vector<GLenum> errs;

        while ((err = glGetError()) != GL_NO_ERROR) {
            errs.emplace_back(err);
        }

        std::string msg = prefix + "\n";

        if (!errs.empty()) {
            for (auto &e : errs)
                msg += ErrorToString(e) + "\n";

            msg = msg.substr(0, msg.size() - 1);

            qCritical() << QString::fromStdString(msg);
        }
    }
}