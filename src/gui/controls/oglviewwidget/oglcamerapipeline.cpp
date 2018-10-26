//
// Created by Jon on 03/09/2018.
//

#include "oglcamerapipeline.h"

const Matrix4f OGLCameraPipeline::getV()
{
    Matrix4f CameraTranslationTrans, CameraRotateTrans;

    CameraTranslationTrans.InitTranslationTransform(-_camPos.x, -_camPos.y, -_camPos.z);
    CameraRotateTrans.InitCameraTransform(_camTarget, _camUp);

    return CameraRotateTrans * CameraTranslationTrans;
}

const Matrix4f OGLCameraPipeline::getM()
{
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans;

    ScaleTrans.InitScaleTransform(_worldScale.x, _worldScale.y, _worldScale.z);
    RotateTrans.InitRotateTransform(_worldRot.x, _worldRot.y, _worldRot.z, _rotateCentre.x, _rotateCentre.y, _rotateCentre.z);
    TranslationTrans.InitTranslationTransform(_worldTrans.x, _worldTrans.y, _worldTrans.z);

    return TranslationTrans * RotateTrans * ScaleTrans;
}

const Matrix4f OGLCameraPipeline::getP()
{
    if (_projMode == ViewMode::Perspective)
        return getPerspP();
    else
        return getOrthoP();
}

const Matrix4f OGLCameraPipeline::getPerspP()
{
    Matrix4f ProjTrans;
    ProjTrans.InitPersProjTransform(_persProjInfo);
    return ProjTrans;
}

const Matrix4f OGLCameraPipeline::getOrthoP()
{
    Matrix4f ProjTrans;
    ProjTrans.InitOrthoProjTransform(_orthoProjInfo);
    return ProjTrans;
}

const Matrix4f OGLCameraPipeline::getMV()
{
    return getV() * getM();
}

const Matrix4f OGLCameraPipeline::getVP()
{
    return getP() * getV();
}

const Matrix4f OGLCameraPipeline::getOrthoVP()
{
    return getOrthoP() * getV();
}

const Matrix4f OGLCameraPipeline::getPerspVP()
{
    return getPerspP() * getV();
}

const Matrix4f OGLCameraPipeline::getMVP()
{
    return getP() * getMV();
}

const Matrix4f OGLCameraPipeline::getOrthoMVP()
{
    return getOrthoP() * getMV();
}

const Matrix4f OGLCameraPipeline::getPerspMVP()
{
    return getPerspP() * getMV();
}

void OGLCameraPipeline::initOrthoProjection(float t, float l, float b, float r, float n, float f)
{
    _orthoProjInfo.t = t;
    _orthoProjInfo.b = b;
    _orthoProjInfo.l = l;
    _orthoProjInfo.r = r;
    _orthoProjInfo.f = f;
    _orthoProjInfo.n = n;
}

void OGLCameraPipeline::initPerspProjection(float fov, float width, float height, float n, float f)
{
    _persProjInfo.FOV = fov;
    _persProjInfo.Height = height;
    _persProjInfo.Width = width;
    _persProjInfo.zNear = n;
    _persProjInfo.zFar = f;
}








