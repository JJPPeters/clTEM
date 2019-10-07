//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_TECHNIQUE_H
#define CLTEM_TECHNIQUE_H

#include <QtOpenGL>
#include "oglmaths.h"

#include <Eigen/Dense>

namespace PGL {
    class Technique {
    public:
        Technique();

        virtual ~Technique();

        virtual void Init();

        void Enable();

        void Disable();

        virtual void Render(const Matrix4f &MV, const Matrix4f &P, const Vector2f &ScreenSize) = 0;

        Eigen::Matrix<float, 3, 2> GetLimits() { return _limits; }

    protected:
        void CompileShaderFromString(GLenum ShaderType, std::string shader);

        void CompileShaderFromFile(GLenum ShaderType, std::string filepath);

        void Finalise();

        GLuint GetUniformLocation(std::string name);

        GLint GetAttribLocation(std::string name);

        void SetProj(const Matrix4f& P);
        void SetModelView(const Matrix4f& MV);

        GLuint _shaderProg;

        GLuint _PLocation;
        GLuint _MVLocation;

        Eigen::Matrix<float, 3, 2> _limits;

    private:
        typedef std::list<GLuint> ShaderObjectList;
        ShaderObjectList _shaderObjectList;
    };
}

#endif //CLTEM_TECHNIQUE_H
