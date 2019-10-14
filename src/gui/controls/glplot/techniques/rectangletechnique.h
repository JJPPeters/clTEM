//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_RECTANGLETECHNIQUE_H
#define CLTEM_RECTANGLETECHNIQUE_H


#include "technique.h"
#include "arraybuffer.h"
#include "attributebuffer.h"
#include "rectangleshader.h"

#include <Eigen/Dense>
#include "oglmaths.h"

#include <memory>

namespace PGL {
    enum Plane {
        x,
        y,
        z
    };
}

namespace PGL {
    class Rectangle : public PGL::Technique {

    public:
        Rectangle(std::shared_ptr<RectangleShader> shdr, float t, float l, float b, float r, float z, Vector4f &colour, PGL::Plane pl);

        ~Rectangle() override {
            if (_positionBuffer)
                _positionBuffer->Delete();

            if (_indexBuffer)
                _indexBuffer->Delete();
        }

        void makeBuffers(std::vector<Vector3f> &positions, Vector4f &col);

        void render(const Matrix4f &MV, const Matrix4f &P, float pix_size) override;

    private:
        std::shared_ptr<RectangleShader> _shader;

        std::shared_ptr<AttributeBuffer> _positionBuffer;
        std::shared_ptr<ArrayBuffer> _indexBuffer;

        Vector4f _col;
        Vector3f _mins, _maxs;
    };
}

#endif //CLTEM_RECTANGLETECHNIQUE_H
