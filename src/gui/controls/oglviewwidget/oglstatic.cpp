//
// Created by Jon on 03/09/2018.
//

#include <QtCore>
#include "oglstatic.h"

std::string OGLErrorToString(GLenum err) {
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
        default:
            return "GL_NO_ERROR";
    }
}

void OGLCheckErrors(std::string prefix) {
    GLenum err;
    std::vector<GLenum> errs;

    while ((err = glGetError()) != GL_NO_ERROR) {
        errs.emplace_back(err);
    }

    std::string msg = prefix + "\n";

    if (!errs.empty()) {
        for (auto &e : errs)
            msg += OGLErrorToString(e) + "\n";

        msg = msg.substr(0, msg.size() - 1);

        qCritical() << QString::fromStdString(msg);
    }
}