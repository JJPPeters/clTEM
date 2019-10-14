//
// Created by Jon on 03/09/2018.
//

#include <iostream>
#include "rectangletechnique.h"



namespace PGL {

    Rectangle::Rectangle(std::shared_ptr<RectangleShader> shader,
                         float t, float l, float b, float r, float z,
                         Eigen::Vector4f &colour, PGL::Plane pl) : Technique() {

        _shader = shader;

        std::vector<Eigen::Vector3f> pos;

        if (pl == PGL::Plane::x)
            pos = {Eigen::Vector3f(z, l, t), Eigen::Vector3f(z, l, b), Eigen::Vector3f(z, r, b), Eigen::Vector3f(z, r, t)};
        else if (pl == PGL::Plane::y)
            pos = {Eigen::Vector3f(l, z, t), Eigen::Vector3f(l, z, b), Eigen::Vector3f(r, z, b), Eigen::Vector3f(r, z, t)};
        else if (pl == PGL::Plane::z)
            pos = {Eigen::Vector3f(l, t, z), Eigen::Vector3f(l, b, z), Eigen::Vector3f(r, b, z), Eigen::Vector3f(r, t, z)};

        makeBuffers(pos, colour);

        _col = colour;
    }

    void Rectangle::makeBuffers(std::vector<Eigen::Vector3f> &positions, Eigen::Vector4f &col) {
        _positionBuffer = std::make_shared<AttributeBuffer>(positions, static_cast<GLuint>(_shader->_posBufLocation));
        std::vector<unsigned int> els = {0, 1, 2, 2, 3, 0}; // TODO: this could be user defined...
        _indexBuffer = std::make_shared<ArrayBuffer>(els, GL_ELEMENT_ARRAY_BUFFER);

        _mins = positions[0];
        _maxs = positions[2];

        _limits << positions[0][0], positions[2][0], positions[2][1], positions[0][1], positions[2][2], positions[0][2];
    }

    void Rectangle::render(const Eigen::Matrix4f &MV, const Eigen::Matrix4f &P, float pix_size) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        _shader->enable();
        _shader->setModelView(MV);
        _shader->setProj(P);

        // Set the limits of the rectangle
        glFuncs->glUniform3f(_shader->_minsLocation, _mins[0], _mins[1], _mins[2]);
        glFuncs->glUniform3f(_shader->_maxsLocation, _maxs[0], _maxs[1], _maxs[2]);
        //
        glFuncs->glUniform1f(_shader->_pixelSizeLocation, pix_size);
        // Set the colour
        glFuncs->glUniform4f(_shader->_colLocation, _col[0], _col[1], _col[2], _col[3]);

        _positionBuffer->Bind();
        _indexBuffer->Bind();

        glFuncs->glDrawElements(GL_TRIANGLES, 6, _indexBuffer->getType(), nullptr);

        _positionBuffer->Unbind();
        _indexBuffer->Unbind();

        _shader->disable();
    }

}