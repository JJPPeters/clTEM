//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_TECHNIQUE_H
#define CLTEM_TECHNIQUE_H

#include <QtOpenGL>
#include "oglmaths.h"

#include <Eigen/Dense>

class AutoShaderResource;

namespace PGL {
    class Technique {
    public:
        explicit Technique(bool visible = true);

        virtual ~Technique();

        void Init();

        void Enable();

        void Disable();

        void setVisible(bool visible) { _visible = visible; }
        bool getVisible() { return _visible; }

        virtual void Render(const Matrix4f &MV, const Matrix4f &P, float pix_size) = 0;

        Eigen::Matrix<float, 3, 2> GetLimits() {
            if (_visible)
                return _limits;
            else
                return Eigen::Matrix<float, 3, 2>::Zero();
        }

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

        bool _visible;
    };
}


//// Singleton to auto call clfftteardown on program termination
class AutoShaderResource
{
public:
    AutoShaderResource() {Q_INIT_RESOURCE(shaders);};
    AutoShaderResource &operator=(AutoShaderResource const &rhs) = delete;

    AutoShaderResource(AutoShaderResource const& copy) = delete;

    ~AutoShaderResource() { Q_CLEANUP_RESOURCE(shaders); }
    inline static AutoShaderResource& GetInstance() { static AutoShaderResource instance; return instance; }
};


#endif //CLTEM_TECHNIQUE_H
