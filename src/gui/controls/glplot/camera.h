//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_CAMERA_H
#define CLTEM_CAMERA_H


#include "camerapipeline.h"
#include <QOpenGLFunctions>
#include <iostream>

enum KeyPress
{
    Up,
    Down,
    Left,
    Right
};

namespace PGL {
    class Camera : public CameraPipeline {
    public:
        Camera(ViewMode Mode = ViewMode::Orthographic);

        Camera(const Vector3f &Pos, const Vector3f &Target, const Vector3f &Up, const Vector3f &Origin,
               const Vector3f &rot, const Vector3f &rot_cent, ViewMode Mode);

        void OnMouseRotate(float dx, float dy);

        void OnMousePan(float dx, float dy);

        void OnScroll(float delta, float pos_frac_x, float pos_frac_y);

        bool OnKeyboardNudge(KeyPress pressed);

        void setWidthHeight(float width, float height);

//        float getOrthoViewHeight() { return (_orthoProjInfo.t - _orthoProjInfo.b); }
//
//        float getOrthoViewWidth() { return (_orthoProjInfo.r - _orthoProjInfo.l); }

        float getWidth() { return _width; }

        float getHeight() { return _height; }

        Vector2f GetScreenSize() {
            return Vector2f(_width, _height);
        }

        void setAspectRatio(float ratio) { _aspect_ratio = ratio; }
        float getAspectRatio() { return _aspect_ratio; }

        void setPixelRatio(float ratio) { _pixel_ratio = ratio; }
        float getPixelRatio() { return _pixel_ratio; }

        float getPixelSize() {
            float px = (_orthoProjInfo.r - _orthoProjInfo.l) / _width;
            float py = (_orthoProjInfo.t - _orthoProjInfo.b) / _height;

            if (px - py > 0.00001)
                throw std::runtime_error("Error, pixel ratio is not equal (" + std::to_string(px) + " to " + std::to_string(py) + ")");

            return px;
        }

//        void ResetViewPort() {
//            QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
//            glFuncs->initializeOpenGLFunctions();
//
//            glFuncs->glViewport(0, 0, _width, _height);
//        }
//
//        void SetViewPort(int x, int y, int w, int h) {
//            QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
//            glFuncs->initializeOpenGLFunctions();
//
//            glFuncs->glViewport(x, y, w, h);
//        }
//
//        void SetViewPortFraction(int x, int y, float frac, float minLim) {
//            float sz = frac * std::min(_width, _height);
//
//            sz = std::max(sz, minLim);
//
//            QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
//            glFuncs->initializeOpenGLFunctions();
//
//            glFuncs->glViewport(x, y, sz, sz);
//        }

    private:

        float _width, _height, _aspect_ratio, _pixel_ratio;
    };
}

#endif //CLTEM_CAMERA_H
