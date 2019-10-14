//
// Created by Jon on 03/09/2018.
//

#include <iostream>
#include "scattertechnique.h"

namespace PGL {

    Scatter::Scatter(std::shared_ptr<ScatterShader> shader, std::vector<Vector3f> pos, std::vector<Vector3f> col) {
        _shader = shader;

        makeBuffers(pos, col);
    }

    void Scatter::makeBuffers(std::vector<Vector3f>& positions, std::vector<Vector3f>& colours)
    {

        for (auto &p: positions) {
            if (p[0] < _limits(0, 0))
                _limits(0, 0) = p[0];
            if (p[0] > _limits(0, 1))
                _limits(0, 1) = p[0];
            // y
            if (p[1] < _limits(1, 0))
                _limits(1, 0) = p[1];
            if (p[1] > _limits(1, 1))
                _limits(1, 1) = p[1];
            // z
            if (p[2] < _limits(2, 0))
                _limits(2, 0) = p[2];
            if (p[2] > _limits(2, 1))
                _limits(2, 1) = p[2];
        }

        _colourBuffer = std::make_shared<AttributeBuffer>(AttributeBuffer(colours, static_cast<GLuint>(_shader->_colBufLocation)));
        _positionBuffer = std::make_shared<AttributeBuffer>(AttributeBuffer(positions, static_cast<GLuint>(_shader->_posBufLocation)));
    }

    void Scatter::render(const Matrix4f &MV, const Matrix4f &P, float pix_size)
    {

        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        _shader->enable();
        _shader->setModelView(MV);
        _shader->setProj(P);

        _colourBuffer->Bind();
        _positionBuffer->Bind();

        glFuncs->glDrawArrays(GL_POINTS, 0, _positionBuffer->getSize());

        _colourBuffer->Unbind();
        _positionBuffer->Unbind();

        _shader->disable();
    }

}