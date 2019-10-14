//
// Created by Jon on 03/09/2018.
//

#include "camera.h"

#include <iostream>

const static float SCROLL_STEP_SCALE = 0.002f;
const static float ZOOM_LIMIT = 0.05f;
const static float STEP_SCALE = 1.0f;

namespace PGL {
    Camera::Camera(ViewMode Mode) {

        _camPos = Eigen::Vector3f(0, 0, 100);

        _camTarget = Eigen::Vector3f(0, 0, 0);
        _camTarget.normalize();

        _camUp = Eigen::Vector3f(0, 1, 0);
        _camUp.normalize();

        _camRight = _camUp.cross(_camTarget);
        _camRight.normalize();

        _projMode = Mode;

        _worldScale = Eigen::Vector3f(1.0, 1.0, 1.0);
        _worldRot = Eigen::Vector3f(0, 0, 0);
        _worldTrans = Eigen::Vector3f(0, 0, 0);
        _rotateCentre = Eigen::Vector3f(0, 0, 0);

        _pixel_ratio = 1.0f;
        _aspect_ratio = 1.0f;
    }

    Camera::Camera(const Eigen::Vector3f &Pos, const Eigen::Vector3f &Target, const Eigen::Vector3f &Up, const Eigen::Vector3f &Origin,
                   const Eigen::Vector3f &rot, const Eigen::Vector3f &rot_cent, ViewMode Mode) {
        _camPos = Pos;

        _camTarget = Target;
        _camTarget.normalize();

        _camUp = Up;
        _camUp.normalize();

        _camRight = _camUp.cross(_camTarget);
        _camRight.normalize();

        _projMode = Mode;

        _worldScale = Eigen::Vector3f(1.0, 1.0, 1.0);
        _worldRot = Eigen::Vector3f(rot[0], rot[1], rot[2]);
        _worldTrans = rot_cent; // this defaults the camera to looking at the rotation centre
        _rotateCentre = Eigen::Vector3f(rot_cent[0], rot_cent[1], rot_cent[2]);

        _pixel_ratio = 1.0f;
        _aspect_ratio = 1.0f;
    }

    void Camera::OnMouseRotate(float dx, float dy) {
        // TODO: I need to clip these vales to avoid precision problems
        _worldRot[2] -= dx;

        _worldRot += _camRight * dy;
    }

    void Camera::OnMousePan(float dx, float dy) {
        float scaling_x, scaling_y;

        if (_projMode == ViewMode::Orthographic) {
            float w = _orthoProjInfo.r - _orthoProjInfo.l;
            float h = _orthoProjInfo.t - _orthoProjInfo.b;

            scaling_x = w / _width;
            scaling_y = h / _height;

        } else {

            Eigen::Vector4f p = Eigen::Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
            Eigen::Vector4f p1 = getMVP() * p;

            auto cr = getCameraRight();
            Eigen::Vector4f right4(cr[0], cr[1], cr[2], 1.0f);
            Eigen::Vector4f p2 = getMVP() * right4;

            if (p1[3] == 0 || p2[3] == 0) {
                scaling_x = 0;
                scaling_y = 0;
            } else {
                // here we calculate the "pixel positions" of the origin and origin + right
                // (not true pixel position as we would need the window x,y coords to add to them)
                // we are only taking the difference so it is all good :D

                // so we end up with the size of a unit vector, at the origin, in pixels
                // then I fudge some factors until it seems to work OK-ish

                // w is the fourth element?
                float f = 1 / p1[3];
                p1 *= f;

                f = 1 / p2[3];
                p2 *= f;

                float x1 = (p1[0]) * _width;
                float y1 = (p1[1]) * _height;

                float x2 = (p2[0]) * _width;
                float y2 = (p2[1]) * _height;

                float ff = std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));

                ff = 0.8f / ff;

                // TODO: this will not work? (need different x and y scalings)
                scaling_x = ff;
                scaling_y = ff;
            }
        }

        // need to calculate right dir from up and target

        Eigen::Vector3f dx_dir = _camRight * dx * scaling_x * STEP_SCALE;
        Eigen::Vector3f dy_dir = _camUp * dy * scaling_y * STEP_SCALE;

        _worldTrans += dx_dir - dy_dir;
    }

    void Camera::OnScroll(float delta, float pos_frac_x, float pos_frac_y) {
        if (_projMode == ViewMode::Orthographic) {
            float w = _orthoProjInfo.r - _orthoProjInfo.l;
            float h = _orthoProjInfo.t - _orthoProjInfo.b;

            float aspect = _width / _height;

            float scaling = std::max(w, h) * SCROLL_STEP_SCALE;

            float new_w, new_h;
            if (aspect > 1.0f) {
                new_w = w - scaling * delta;
                if (new_w < ZOOM_LIMIT)
                    new_w = ZOOM_LIMIT;

                new_h = new_w / aspect;
            } else {
                new_h = h - scaling * delta;
                if (new_h < ZOOM_LIMIT)
                    new_h = ZOOM_LIMIT;

                new_w = new_h * aspect;
            }

            _orthoProjInfo.l += (w - new_w) * pos_frac_x;
            _orthoProjInfo.r = _orthoProjInfo.l + new_w;

            _orthoProjInfo.b += (h - new_h) * pos_frac_y;
            _orthoProjInfo.t = _orthoProjInfo.b + new_h;

        } else
            _worldTrans[0] -= (delta * SCROLL_STEP_SCALE);
    }

    bool Camera::OnKeyboardNudge(KeyPress pressed) {
        bool Ret = false;

        switch (pressed) {
            case KeyPress::Up: {
                _camPos -= (_camUp * STEP_SCALE);
                Ret = true;
            }
                break;
            case KeyPress::Down: {
                _camPos += (_camUp * STEP_SCALE);
                Ret = true;
            }
                break;
            case KeyPress::Left: {
                _camPos += _camRight * STEP_SCALE;
                Ret = true;
            }
                break;
            case KeyPress::Right: {
                _camPos -= _camRight * STEP_SCALE;
                Ret = true;
            }
                break;
        }

        return Ret;
    }

    void Camera::setWidthHeight(float width, float height) {
        if (_projMode == ViewMode::Orthographic) {

            float aspect_new = width / height;

            float w = _orthoProjInfo.r - _orthoProjInfo.l;
            float h = _orthoProjInfo.t - _orthoProjInfo.b;

            // special case for when the aspect ratio crosses 1
            if (aspect_new <= 1 and getAspectRatio() >= 1)
                w = h;
            else if (aspect_new > 1 and getAspectRatio() <= 1)
                h = w;

            float view_width, view_height;

            if (aspect_new <= 1) {
                view_width = w;
                view_height = view_width / aspect_new;
            } else {
                view_height = h;
                view_width = view_height * aspect_new;
            }

            float mid_x = (_orthoProjInfo.r + _orthoProjInfo.l) / 2.0f;
            float mid_y = (_orthoProjInfo.t + _orthoProjInfo.b) / 2.0f;

            float t = mid_y + view_height / 2.0f;
            float l = mid_x - view_width / 2.0f;
            float b = mid_y - view_height / 2.0f;
            float r = mid_x + view_width / 2.0f;

            setOrthoProjection(t, l, b, r);

        } else {
            _persProjInfo.Width = _width;
            _persProjInfo.Height = _height;
        }

        _width = width;
        _height = height;
    }
}