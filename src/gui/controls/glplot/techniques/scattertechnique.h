//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_SCATTERTECHNIQUE_H
#define CLTEM_SCATTERTECHNIQUE_H


#include "technique.h"
#include "arraybuffer.h"
#include "attributebuffer.h"
#include "scattershader.h"

#include "error.h"

#include <memory>

namespace PGL {
    class Scatter : public PGL::Technique {
    public:
        Scatter(std::shared_ptr<ScatterShader> shader, std::vector<Eigen::Vector3f> pos, std::vector<Eigen::Vector3f> col);

        ~Scatter() override {
            if (_positionBuffer)
                _positionBuffer->Delete();

            if (_colourBuffer)
                _colourBuffer->Delete();
        }

        void makeBuffers(std::vector<Eigen::Vector3f> &positions, std::vector<Eigen::Vector3f> &colours);

        void render(const Eigen::Matrix4f &MV, const Eigen::Matrix4f &P, float pix_size);

    private:
        std::shared_ptr<ScatterShader> _shader;

        std::shared_ptr<AttributeBuffer> _positionBuffer, _colourBuffer;

    };
}

#endif //CLTEM_SCATTERTECHNIQUE_H
