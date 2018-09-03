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
    OGLCamera(const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up, const Vector3f& Origin, float rx, float ry, float rz, ViewMode Mode);

    void OnMouseLeft(float dx, float dy);

    void OnMouseRight(float dx, float dy);

    void OnScroll(float delta);

    bool OnKeyboard(KeyPress pressed);

    void InitPerspProjection(float fov, float n, float f);

    void SetWidthHeight(float width, float height);

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
