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

//        glDeleteShader();

        Q_CLEANUP_RESOURCE(shaders);
    }

    bool Init() override;

    void MakeRect(float t, float l, float b, float r, float z, Vector4f col);
    void MakeBuffers(std::vector<Vector3f> &positions, Vector4f &col, Vector4f &lims);

    bool Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize);

    void SetLims(const Vector4f &lims);
    void SetCol(const Vector4f& col);

private:
    std::shared_ptr<OGLAttributeBuffer> _positionBuffer;
    std::shared_ptr<OGLArrayBuffer> _indexBuffer;

    bool _haveBuffers;

    Vector4f _col, _lims;

//    GLuint
    GLint _posBufLocation, _colLocation, _limsLocation;
};


#endif //CLTEM_OGLRECTANGLETECHNIQUE_H
