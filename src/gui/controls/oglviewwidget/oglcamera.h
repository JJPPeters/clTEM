//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_OGLCAMERA_H
#define CLTEM_OGLCAMERA_H


#include "oglcamerapipeline.h"
#include <QOpenGLFunctions>

enum KeyPress
{
    Up,
    Down,
    Left,
    Right
};

class OGLCamera : public OGLCameraPipeline
{
public:
    OGLCamera(const Vector3f &Pos, const Vector3f &Target, const Vector3f &Up, const Vector3f &Origin,
                  const Vector3f &rot, const Vector3f &rot_cent, ViewMode Mode);

    void OnMouseRotate(float dx, float dy);

    void OnMousePan(float dx, float dy);

    void OnScroll(float delta);

    bool OnKeyboardNudge(KeyPress pressed);

    void InitPerspProjection(float fov, float n, float f);

    void SetWidthHeight(float width, float height);

    float GetOrthoViewHeight() { return (_orthoProjInfo.t - _orthoProjInfo.b);}
    float GetOrthoViewWidth() { return (_orthoProjInfo.r - _orthoProjInfo.l);}

    float GetWidth(){ return _width; }
    float GetHeight(){ return _height; }

    const Vector2f GetScreenSize()
    {
        return Vector2f(_width, _height);
    }

    void SetPixelRatio(float ratio)
    {
        _ratio = ratio;
    }

    void ResetViewPort()
    {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glViewport(0, 0, _width, _height);
    }

    void SetViewPort(int x, int y, int w, int h)
    {
        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glViewport(x, y, w, h);
    }

    void SetViewPortFraction(int x, int y, float frac, float minLim)
    {
        float sz = frac * std::min(_width, _height);

        sz = std::max(sz, minLim);

        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        glFuncs->glViewport(x, y, sz, sz);
    }

private:

    float _width, _height, _ratio;
};

#endif //CLTEM_OGLCAMERA_H
