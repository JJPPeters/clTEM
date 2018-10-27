#include <utility>

#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QWidget>
#include <QtWidgets>
#include <QOpenGLWidget>
#include <QtOpenGL>

#include <memory>

#include "GL/glu.h"

#include "oglarraybuffer.h"
#include "oglbillboardtechnique.h"
#include "oglcamera.h"
#include "oglrectangletechnique.h"

namespace View {
    enum Direction {
        Front,
        Back,
        Left,
        Right,
        Top,
        Bottom
    };
}

namespace OGL {
    enum Plane {
        x,
        y,
        z
    };
}

class OGLViewWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit OGLViewWidget(QWidget *parent);
    ~OGLViewWidget() override;

    void setDrawRects(bool v) {_drawRects = v;}
    bool getDrawRects() {return _drawRects;}

    void clearRectBuffers() {_recSlices.clear();}

    void PlotAtoms(std::vector<Vector3f> pos, std::vector<Vector3f> cols, View::Direction view_dir,
            float x_min,
            float x_max,
            float y_min,
            float y_max,
            float z_min,
            float z_max) {
        // TODO: centre on structure
        // TODO: get limits of view and show them

        MakeScatterBuffers(pos, cols);

        auto x_offset = -(x_max + x_min) / 2;
        auto y_offset = -(y_max + y_min) / 2;
        auto z_offset = -(z_max + z_min) / 2;
        _rotation_offset = Vector3f(x_offset, y_offset, z_offset);

        SetCube(x_min, x_max, y_min, y_max, z_min, z_max);

        SetViewDirection(view_dir);
    }

    void AddRectBuffer(float t, float l, float b, float r, float z, Vector4f &colour, OGL::Plane pl) {
        makeCurrent();
        auto rec = std::make_shared<OGLRectangleTechnique>();
        rec->Init();

        std::vector<Vector3f> pos;

        if (pl == OGL::Plane::x)
            pos = {Vector3f(z, l, t), Vector3f(z, l, b), Vector3f(z, r, b), Vector3f(z, r, t)};
        else if (pl == OGL::Plane::y)
            pos = {Vector3f(l, z, t), Vector3f(l, z, b), Vector3f(r, z, b), Vector3f(r, z, t)};
        else if (pl == OGL::Plane::z)
            pos = {Vector3f(l, t, z), Vector3f(l, b, z), Vector3f(r, b, z), Vector3f(r, t, z)};

        rec->MakeBuffers(pos, colour, pos[0], pos[2]);

        _recSlices.push_back(rec);
        doneCurrent();
    }

    void SetViewDirection(View::Direction view_dir) {
        // TODO: might need to sort these vectors out
        auto v_d = directionEnumToVector(view_dir);
        Vector3f n_d = directionEnumToVector(View::Direction::Bottom);
        if (view_dir == View::Direction::Top)
            n_d = directionEnumToVector(View::Direction::Back);
        else if (view_dir == View::Direction::Bottom)
            n_d = directionEnumToVector(View::Direction::Right);

        SetCamera(v_d*-1, v_d, n_d, ViewMode::Orthographic);

        // cube coords need to be defined for this to work
        fitView(1.0);
    }

    std::shared_ptr<OGLCamera> GetCamera() { return _camera;}

    void SetCube(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);
    void SetCube(std::vector<Vector3f> Cube) {
        makeCurrent();
        _cubeCoords = std::move(Cube);
        doneCurrent();
    }

    void fitView(float extend = 1.0);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    Vector3f directionEnumToVector(View::Direction d);

    std::shared_ptr<OGLBillBoardTechnique> _technique;
    std::vector<std::shared_ptr<OGLRectangleTechnique>> _recSlices;

    std::shared_ptr<OGLCamera> _camera;

    std::vector<Vector3f> _cubeCoords;

    bool _drawRects;

    // width of the openGL window
    float _width, _height;

    // Structure limits
    Vector3f _rotation_offset;

    Vector3f _background;

    QPoint _lastPos;

    void MakeScatterBuffers(std::vector<Vector3f> &positions, std::vector<Vector3f> &colours)
    {
        if(positions.size() != colours.size())
            throw std::runtime_error("OpenGL: Scatter position vector size does not match scatter colour vector size");

        makeCurrent();
        _technique->MakeBuffers(positions, colours);
        doneCurrent();
    }

    void SetCamera(Vector3f position, Vector3f target, Vector3f up, Vector3f rot, ViewMode mode);
    void SetCamera(Vector3f position, Vector3f target, Vector3f up, ViewMode mode);

    void fitPerspView(float extend = 1.0);
    void fitOrthoView(float extend = 1.0);
};

#endif // MYGLWIDGET_H
