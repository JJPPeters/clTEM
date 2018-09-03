//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLTEXTURE_H
#define CLTEM_OGLTEXTURE_H


#include <QtOpenGL>

///
/// \brief The OGLTexture class, more or less a rip off of ogldev_texture (GPLv3)
///

class OGLTexture
{
public:
    explicit OGLTexture(GLenum TextureTarget);

    bool LoadCharArrayFromFile(std::string filename, int width, int height);

    void Bind(GLenum TextureUnit);

private:
    GLenum _textureTarget;
    GLuint _textureObject;
};

#endif //CLTEM_OGLTEXTURE_H
