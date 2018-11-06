//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLBILLBOARDTECHNIQUE_H
#define CLTEM_OGLBILLBOARDTECHNIQUE_H


#include "ogltechnique.h"
#include "oglmaths.h"
#include "oglarraybuffer.h"
#include "ogltexture.h"
#include "oglattributebuffer.h"
#include "oglstatic.h"

#include <memory>

class OGLBillBoardTechnique : public OGLTechnique
{
public:
    OGLBillBoardTechnique();

    ~OGLBillBoardTechnique() override {
        if (_positionBuffer)
            _positionBuffer->Delete();
        if (_colourBuffer)
            _colourBuffer->Delete();

        Q_CLEANUP_RESOURCE(shaders);
    }

    bool Init() override;

    void MakeBuffers(std::vector<Vector3f>& positions, std::vector<Vector3f>& colours);

    bool Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize);

private:
    std::shared_ptr<OGLAttributeBuffer> _positionBuffer, _colourBuffer;

    bool _haveBuffers;

    GLint _posBufLocation;
    GLint _colBufLocation;
};


#endif //CLTEM_OGLBILLBOARDTECHNIQUE_H
