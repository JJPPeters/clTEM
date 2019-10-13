//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_SCATTERTECHNIQUE_H
#define CLTEM_SCATTERTECHNIQUE_H


#include "technique.h"
#include "oglmaths.h"
#include "arraybuffer.h"
//#include "ogltexture.h"
#include "attributebuffer.h"
#include "error.h"

#include <memory>

namespace PGL {
    class Scatter : public PGL::Technique {
    public:
        Scatter();

        Scatter(std::vector<Vector3f> pos, std::vector<Vector3f> col);

        ~Scatter() override {
            if (_positionBuffer)
                _positionBuffer->Delete();

            if (_colourBuffer)
                _colourBuffer->Delete();
        }

        void Init();

        void MakeBuffers(std::vector<Vector3f> &positions, std::vector<Vector3f> &colours);

        void Render(const Matrix4f &MV, const Matrix4f &P, float pix_size);

    private:
        std::shared_ptr<AttributeBuffer> _positionBuffer, _colourBuffer;

        bool _haveBuffers;

        GLint _posBufLocation;
        GLint _colBufLocation;
    };
}

#endif //CLTEM_SCATTERTECHNIQUE_H
