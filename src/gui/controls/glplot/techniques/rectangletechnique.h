//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_RECTANGLETECHNIQUE_H
#define CLTEM_RECTANGLETECHNIQUE_H


#include "technique.h"
#include "oglmaths.h"
#include "arraybuffer.h"
#include "attributebuffer.h"

#include <memory>

namespace PGL {
    class RectangleTechnique : public PGL::Technique {

    public:
        RectangleTechnique();

        ~RectangleTechnique() override {
            if (_positionBuffer)
                _positionBuffer->Delete();

            Q_CLEANUP_RESOURCE(shaders);
        }

        void Init() override;

        void MakeBuffers(std::vector<Vector3f> &positions, Vector4f &col, Vector3f &mins, Vector3f &maxs);

        void Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize);

        void SetLims(const Vector3f &mins, const Vector3f &maxs);

        void SetCol(const Vector4f &col);

    private:
        std::shared_ptr<AttributeBuffer> _positionBuffer;
        std::shared_ptr<ArrayBuffer> _indexBuffer;

        bool _haveBuffers;

        Vector4f _col;
        Vector3f _mins, _maxs;

//    GLuint
        GLint _posBufLocation, _colLocation, _minsLocation, _maxsLocation;
    };
}

#endif //CLTEM_RECTANGLETECHNIQUE_H
