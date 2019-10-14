//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_CAMERAPIPELINE_H
#define CLTEM_CAMERAPIPELINE_H

#include "static.h"
#include <Eigen/Dense>

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

        const Eigen::Vector3f &getCameraPos() { return _camPos; }

        const Eigen::Vector3f &getCameraTarget() const { return _camTarget; }

        const Eigen::Vector3f &getCameraUp() { return _camUp; }

        const Eigen::Vector3f &getCameraRight() { return _camRight; }

        const Eigen::Vector3f &getWorldRotation() { return _worldRot; }

        void setCameraPos(Eigen::Vector3f Pos) { _camPos = Pos; }

        void setPerspProjection(float fov, float width, float height, float n, float f);

        void setOrthoProjection(float t, float l, float b, float r, float n = -100.0f, float f = 10000.0f);

        const Eigen::Matrix4f getV();

        const Eigen::Matrix4f getM();

        const Eigen::Matrix4f getP();

        const Eigen::Matrix4f getOrthoP();

        const Eigen::Matrix4f getPerspP();

        const Eigen::Matrix4f getMV();

        const Eigen::Matrix4f getVP();

        const Eigen::Matrix4f getPerspVP();

        const Eigen::Matrix4f getOrthoVP();

        const Eigen::Matrix4f getMVP();

        const Eigen::Matrix4f getPerspMVP();

        const Eigen::Matrix4f getOrthoMVP();

    protected:
        ViewMode _projMode;

        PersProjInfo _persProjInfo;
        OrthoProjInfo _orthoProjInfo;

        Eigen::Vector3f _camPos;
        Eigen::Vector3f _camTarget;
        Eigen::Vector3f _camUp;
        Eigen::Vector3f _camRight;

        Eigen::Vector3f _worldScale;
        Eigen::Vector3f _worldTrans;
        Eigen::Vector3f _worldRot;
        Eigen::Vector3f _rotateCentre;
    };
}

#endif //CLTEM_CAMERAPIPELINE_H
