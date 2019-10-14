//
// Created by Jon on 03/09/2018.
//

#include "camerapipeline.h"

namespace PGL {
    const Eigen::Matrix4f CameraPipeline::getV() {
//        Eigen::Matrix4f CameraTranslationTrans, CameraRotateTrans;
//
//        CameraTranslationTrans.InitTranslationTransform(-_camPos.x, -_camPos.y, -_camPos.z);
//        CameraRotateTrans.InitCameraTransform(_camTarget, _camUp);

        auto CameraTranslationTrans = PGL::initTranslationTransform(-_camPos[0], -_camPos[1], -_camPos[2]);
        auto CameraRotateTrans = PGL::initCameraTransform(_camTarget, _camUp);

        return CameraRotateTrans * CameraTranslationTrans;
    }

    const Eigen::Matrix4f CameraPipeline::getM() {

        auto ScaleTrans = PGL::initScaleTransform(_worldScale[0], _worldScale[1], _worldScale[2]);
        auto RotateTrans = PGL::initRotateTransform(_worldRot[0], _worldRot[1], _worldRot[2],
                                                    _rotateCentre[0], _rotateCentre[1], _rotateCentre[2]);
        auto TranslationTrans = PGL::initTranslationTransform(_worldTrans[0], _worldTrans[1], _worldTrans[2]);

        return TranslationTrans * RotateTrans * ScaleTrans;
    }

    const Eigen::Matrix4f CameraPipeline::getP() {
        if (_projMode == ViewMode::Perspective)
            return getPerspP();
        else
            return getOrthoP();
    }

    const Eigen::Matrix4f CameraPipeline::getPerspP() {
        return PGL::initPersProjTransform(_persProjInfo);
    }

    const Eigen::Matrix4f CameraPipeline::getOrthoP() {
        return PGL::initOrthoProjTransform(_orthoProjInfo);
    }

    const Eigen::Matrix4f CameraPipeline::getMV() {
        return getV() * getM();
    }

    const Eigen::Matrix4f CameraPipeline::getVP() {
        return getP() * getV();
    }

    const Eigen::Matrix4f CameraPipeline::getOrthoVP() {
        return getOrthoP() * getV();
    }

    const Eigen::Matrix4f CameraPipeline::getPerspVP() {
        return getPerspP() * getV();
    }

    const Eigen::Matrix4f CameraPipeline::getMVP() {
        return getP() * getMV();
    }

    const Eigen::Matrix4f CameraPipeline::getOrthoMVP() {
        return getOrthoP() * getMV();
    }

    const Eigen::Matrix4f CameraPipeline::getPerspMVP() {
        return getPerspP() * getMV();
    }

    void CameraPipeline::setOrthoProjection(float t, float l, float b, float r, float n, float f) {
        _orthoProjInfo.t = t;
        _orthoProjInfo.b = b;
        _orthoProjInfo.l = l;
        _orthoProjInfo.r = r;
        _orthoProjInfo.f = f;
        _orthoProjInfo.n = n;
    }

    void CameraPipeline::setPerspProjection(float fov, float width, float height, float n, float f) {
        _persProjInfo.FOV = fov;
        _persProjInfo.Height = height;
        _persProjInfo.Width = width;
        _persProjInfo.zNear = n;
        _persProjInfo.zFar = f;
    }
}







