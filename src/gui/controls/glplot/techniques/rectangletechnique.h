//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_RECTANGLETECHNIQUE_H
#define CLTEM_RECTANGLETECHNIQUE_H


#include "technique.h"
#include "oglmaths.h"
#include "arraybuffer.h"
#include "attributebuffer.h"

#include <Eigen/Dense>

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
        Rectangle(float t, float l, float b, float r, float z, Vector4f &colour, PGL::Plane pl);

        ~Rectangle() override {
            if (_positionBuffer)
                _positionBuffer->Delete();

            if (_indexBuffer)
                _indexBuffer->Delete();
        }

        void Init() override;

        void MakeBuffers(std::vector<Vector3f> &positions, Vector4f &col);

        void Render(const Matrix4f &MV, const Matrix4f &P, float pix_size) override;

    private:
        std::shared_ptr<AttributeBuffer> _positionBuffer;
        std::shared_ptr<ArrayBuffer> _indexBuffer;

        bool _haveBuffers;

        Vector4f _col;
        Vector3f _mins, _maxs;

//    GLuint
        GLint _posBufLocation, _colLocation, _minsLocation, _maxsLocation, _pixelSizeLocation;
    };
}

#endif //CLTEM_RECTANGLETECHNIQUE_H
