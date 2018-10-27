//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLRECTANGLETECHNIQUE_H
#define CLTEM_OGLRECTANGLETECHNIQUE_H


#include "ogltechnique.h"
#include "oglmaths.h"
#include "oglarraybuffer.h"
#include "ogltexture.h"
#include "oglattributebuffer.h"

#include <memory>

class OGLRectangleTechnique : public OGLTechnique
{

public:
    OGLRectangleTechnique();

    ~OGLRectangleTechnique() override {
        if (_positionBuffer)
            _positionBuffer->Delete();

        Q_CLEANUP_RESOURCE(shaders);
    }

    bool Init() override;

    void MakeBuffers(std::vector<Vector3f> &positions, Vector4f &col, Vector3f &mins, Vector3f &maxs);

    bool Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize);

    void SetLims(const Vector3f &mins, const Vector3f & maxs);
    void SetCol(const Vector4f& col);

private:
    std::shared_ptr<OGLAttributeBuffer> _positionBuffer;
    std::shared_ptr<OGLArrayBuffer> _indexBuffer;

    bool _haveBuffers;

    Vector4f _col;
    Vector3f _mins, _maxs;

//    GLuint
    GLint _posBufLocation, _colLocation, _minsLocation, _maxsLocation;
};


#endif //CLTEM_OGLRECTANGLETECHNIQUE_H
