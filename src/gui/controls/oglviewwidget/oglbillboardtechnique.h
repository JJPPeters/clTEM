//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLBILLBOARDTECHNIQUE_H
#define CLTEM_OGLBILLBOARDTECHNIQUE_H


#include "ogltechnique.h"
#include "oglmaths.h"
#include "oglvertexbuffer.h"
#include "ogltexture.h"

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

    void SetProj(const Matrix4f& P);
    void SetModelView(const Matrix4f& MV);
    void SetColourTextureUnit(unsigned int TexterUnit);
    void SetScreenSize(Vector2f size);
    GLint GetAttribLocation(std::string name);

    void MakeBuffers(std::vector<Vector3f>& positions, std::vector<Vector3f>& colours);

    bool Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize);

private:
    std::shared_ptr<OGLVertexBuffer> _positionBuffer, _colourBuffer;

    std::shared_ptr<OGLTexture> _sphereTexture;

    bool _haveBuffers;

    GLint _posBufLocation;
    GLint _colBufLocation;
    GLuint _PLocation;
    GLuint _MVLocation;
//    GLuint _TextureLocation;
    GLuint _ScreenSizeLocation;
};


#endif //CLTEM_OGLBILLBOARDTECHNIQUE_H
