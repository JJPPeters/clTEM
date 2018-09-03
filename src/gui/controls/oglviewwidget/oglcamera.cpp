//
// Created by Jon on 03/09/2018.
//

#include "oglcamera.h"

#include <iostream>

const static float SCROLL_STEP_SCALE = 0.01f;
const static float STEP_SCALE = 1.0f;

OGLCamera::OGLCamera(const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up, const Vector3f& Origin, float rx, float ry, float rz, ViewMode Mode)
{
    _camPos = Pos;

    _camTarget = Target;
    _camTarget.Normalize();

    _camUp = Up;
    _camUp.Normalize();

    _camRight = _camUp.Cross(_camTarget);
    _camRight.Normalize();

    _projMode = Mode;

    _worldScale = Vector3f(1.0, 1.0, 1.0);
    _worldRot = Vector3f(rx, ry, rz);
    _worldTrans = Vector3f(0.0, 0.0, 0.0);
}

void OGLCamera::OnMouseLeft(float dx, float dy)
{
    // TODO: I need to clip these vales to avoid precision problems
    _worldRot.z -= dx;

    _worldRot += _camRight * dy;
}

void OGLCamera::OnMouseRight(float dx, float dy)
{
    float scaling = 0.0f;

    if (_projMode == ViewMode::Orthographic)
    {
        float fov = _orthoProjInfo.t;
        scaling = 2.0f * fov * _ratio / _height;
    }
    else
    {
        Vector4f p = Vector4f(0.0f, 0.0f, 0.0f, 1.0f);

        Vector4f p1 = getMVP() * p;

        Vector4f right4(getCameraRight(), 1.0f);

        Vector4f p2 = getMVP() * (p + right4);

        if (p1.w == 0 || p2.w == 0)
            scaling = 0;
        else
        {
            // here we calculate the "pixel positions" of the origin and origin + right
            // (not true pixel position as we would need the window x,y coords to add to them)
            // we are only taking the difference so it is all good :D

            // so we end up with the size of a unit vector, at the origin, in pixels
            // then I fudge some factors until it seems to work OK-ish

            float f = 1 / p1.w;
            p1 *= f;
            p1.w = f;

            f = 1 / p2.w;
            p2 *= f;
            p2.w = f;

            float x1 = (p1.x * 0.5 + 0.5) * _width;
            float y1 = (p1.y * 0.5 + 0.5) * _height;

            float x2 = (p2.x * 0.5 + 0.5) * _width;
            float y2 = (p2.y * 0.5 + 0.5) * _height;

            float ff = std::sqrt((x1 - x2)*(x1 - x2) + (y1-y2)*(y1-y2));

            ff = 0.8 / ff;

            scaling = ff;
        }
    }

    // need to calculate right dir from up and target

    Vector3f dx_dir = _camRight * dx * scaling * STEP_SCALE;
    Vector3f dy_dir = _camUp * dy * scaling * STEP_SCALE;

    _worldTrans += dx_dir - dy_dir;
}


void OGLCamera::OnScroll(float delta)
{
    if (_projMode == ViewMode::Orthographic)
    {
        float aspect = _width / _height;
        // scroll out gives bigger fov so subtract
        float fov = _orthoProjInfo.t - delta * SCROLL_STEP_SCALE;

        if (fov < 1)
            fov = 1;

        _orthoProjInfo.t = fov;
        _orthoProjInfo.b = -fov;
        _orthoProjInfo.l = -fov * aspect;
        _orthoProjInfo.r = fov * aspect;
    }
    else
        _worldTrans.x -= (delta * SCROLL_STEP_SCALE);
}

bool OGLCamera::OnKeyboard(KeyPress pressed)
{
    bool Ret = false;

    switch (pressed)
    {
        case KeyPress::Up:
        {
            _camPos += (_camUp * STEP_SCALE);
            Ret = true;
        }
            break;
        case KeyPress::Down:
        {
            _camPos -= (_camUp * STEP_SCALE);
            Ret = true;
        }
            break;
        case KeyPress::Left:
        {
            _camPos -= _camRight * STEP_SCALE;
            Ret = true;
        }
            break;
        case KeyPress::Right:
        {
            _camPos += _camRight * STEP_SCALE;
            Ret = true;
        }
            break;
    }

    return Ret;
}

void OGLCamera::SetWidthHeight(float width, float height)
{
    _width = width;
    _height = height;
    _persProjInfo.Width = _width;
    _persProjInfo.Height = _height;
    float t = _orthoProjInfo.t;
    float aspect = width / height;
    _orthoProjInfo.l = -t * aspect;
    _orthoProjInfo.r = t * aspect;
}