//
// Created by Jon on 03/09/2018.
//

#include <iostream>
#include "rectangletechnique.h"



namespace PGL {

    Rectangle::Rectangle(std::shared_ptr<RectangleShader> shader,
                         float t, float l, float b, float r, float z,
                         Vector4f &colour, PGL::Plane pl) : Technique() {

        _shader = shader;

        std::vector<Vector3f> pos;

        if (pl == PGL::Plane::x)
            pos = {Vector3f(z, l, t), Vector3f(z, l, b), Vector3f(z, r, b), Vector3f(z, r, t)};
        else if (pl == PGL::Plane::y)
            pos = {Vector3f(l, z, t), Vector3f(l, z, b), Vector3f(r, z, b), Vector3f(r, z, t)};
        else if (pl == PGL::Plane::z)
            pos = {Vector3f(l, t, z), Vector3f(l, b, z), Vector3f(r, b, z), Vector3f(r, t, z)};

        makeBuffers(pos, colour);

        _col = colour;
    }

    void Rectangle::makeBuffers(std::vector<Vector3f> &positions, Vector4f &col) {
        _positionBuffer = std::make_shared<AttributeBuffer>(positions, static_cast<GLuint>(_shader->_posBufLocation));
        std::vector<unsigned int> els = {0, 1, 2, 2, 3, 0}; // TODO: this could be user defined...
        _indexBuffer = std::make_shared<ArrayBuffer>(els, GL_ELEMENT_ARRAY_BUFFER);

        _mins = positions[0];
        _maxs = positions[2];

        _limits << positions[0].x, positions[2].x, positions[2].y, positions[0].y, positions[2].z, positions[0].z;
    }

    void Rectangle::render(const Matrix4f &MV, const Matrix4f &P, float pix_size) {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        _shader->enable();
        _shader->setModelView(MV);
        _shader->setProj(P);

        // Set the limits of the rectangle
        glFuncs->glUniform3f(_shader->_minsLocation, _mins.x, _mins.y, _mins.z);
        glFuncs->glUniform3f(_shader->_maxsLocation, _maxs.x, _maxs.y, _maxs.z);
        //
        glFuncs->glUniform1f(_shader->_pixelSizeLocation, pix_size);
        // Set the colour
        glFuncs->glUniform4f(_shader->_colLocation, _col.x, _col.y, _col.z, _col.w);

        _positionBuffer->Bind();
        _indexBuffer->Bind();

        glFuncs->glDrawElements(GL_TRIANGLES, 6, _indexBuffer->getType(), nullptr);

        _positionBuffer->Unbind();
        _indexBuffer->Unbind();

        _shader->disable();
    }

}