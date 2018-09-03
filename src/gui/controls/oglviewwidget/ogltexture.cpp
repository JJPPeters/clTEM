//
// Created by Jon on 03/09/2018.
//

#include "ogltexture.h"
#include <fstream>
#include <iterator>
#include <algorithm>

OGLTexture::OGLTexture(GLenum TextureTarget)
{
    _textureTarget = TextureTarget;
}

bool OGLTexture::LoadCharArrayFromFile(std::string filename, int width, int height)
{
    std::ifstream input(filename, std::ios::binary);

    //no new lines
    input.unsetf(std::ios::skipws);

    //get size
    std::streampos fileSize;
    input.seekg(0, std::ios::end);
    fileSize = input.tellg();
    input.seekg(0, std::ios::beg);

    if (width * height * 4 != fileSize)
    {
        throw std::runtime_error("OpenGL: Loaded texture of incompatible size.");
        return false;
    }

    std::vector<unsigned char> data(fileSize);

    data.insert(data.begin(), std::istream_iterator<unsigned char>(input), std::istream_iterator<unsigned char>());

    glGenTextures(1, &_textureObject);
    glBindTexture(_textureTarget, _textureObject);
    //TODO: might be able to do just RGB here (possibly a speed up?
    glTexImage2D(_textureTarget, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);

    glTexParameteri (_textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (_textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri (_textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri (_textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(_textureTarget, 0);

    return true;
}

void OGLTexture::Bind(GLenum TextureUnit)
{
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    glFuncs->glActiveTexture(TextureUnit);

    glBindTexture(_textureTarget, _textureObject);
}
