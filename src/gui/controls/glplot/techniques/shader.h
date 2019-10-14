//
// Created by Jon on 14/10/2019.
//

#ifndef CLTEM_SHADER_H
#define CLTEM_SHADER_H

#include <QtOpenGL>

#include "shaderresource.h"
//#include <Eigen/Dense>

#include "oglmaths.h"

namespace PGL {
    class Shader {
    public:
        Shader();

        ~Shader();

        virtual void initialise();

        void enable();
        void disable();

        // these are convenience function, they are commonly used (by me)
        void setProj(const Matrix4f& P);
        void setModelView(const Matrix4f& MV);

    protected:
        void finalise();

        void compileFromString(GLenum shader_type, std::string shader);

        void compileFromFile(GLenum shader_type, std::string file_path);

        GLuint getUniformLocation(std::string name);

        GLuint getAttribLocation(std::string name);

        GLuint _shaderProg;

        GLuint _PLocation;
        GLuint _MVLocation;

    private:
        typedef std::list<GLuint> ShaderObjectList;
        ShaderObjectList _shaderObjectList;

    };
}

#endif //CLTEM_SHADER_H
