//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_CAMERAPIPELINE_H
#define CLTEM_CAMERAPIPELINE_H


#include "oglmaths.h"

enum ViewMode
{
    Perspective,
    Orthographic
};

// This class is just to contain the brunt work / geometry stuff for cameras
// It doesn't need to be it's own class but it keeps things tidy

// THINGS TO NOTE/REMEMBER

// Model (M) is the transforms of the world and the objects in it
// View (V) is the transforms of the camera
// Perspective (P) is the way this objects are projected onto the screen

// Perspective FOV is horizontal
namespace PGL {
    class CameraPipeline {
    public:
        ViewMode getProjectionMode() { return _projMode; }

        const Vector3f &getCameraPos() { return _camPos; }

        const Vector3f &getCameraTarget() const { return _camTarget; }

        const Vector3f &getCameraUp() { return _camUp; }

        const Vector3f &getCameraRight() { return _camRight; }

        const Vector3f &getWorldRotation() { return _worldRot; }

        void setCameraPos(Vector3f Pos) { _camPos = Pos; }

        void setPerspProjection(float fov, float width, float height, float n, float f);

        void setOrthoProjection(float t, float l, float b, float r, float n = -100.0f, float f = 10000.0f);

        const Matrix4f getV();

        const Matrix4f getM();

        const Matrix4f getP();

        const Matrix4f getOrthoP();

        const Matrix4f getPerspP();

        const Matrix4f getMV();

        const Matrix4f getVP();

        const Matrix4f getPerspVP();

        const Matrix4f getOrthoVP();

        const Matrix4f getMVP();

        const Matrix4f getPerspMVP();

        const Matrix4f getOrthoMVP();

    protected:
        ViewMode _projMode;

        PersProjInfo _persProjInfo;
        OrthoProjInfo _orthoProjInfo;

        Vector3f _camPos;
        Vector3f _camTarget;
        Vector3f _camUp;
        Vector3f _camRight;

        Vector3f _worldScale;
        Vector3f _worldTrans;
        Vector3f _worldRot;
        Vector3f _rotateCentre;
    };
}

#endif //CLTEM_CAMERAPIPELINE_H
