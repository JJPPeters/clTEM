#include <utility>

#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QWidget>
#include <QtWidgets>
#include <QOpenGLWidget>
#include <QtOpenGL>

#include <memory>

#include "GL/glu.h"

#include "oglvertexbuffer.h"
#include "oglbillboardtechnique.h"
#include "oglcamera.h"

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

class OGLViewWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit OGLViewWidget(QWidget *parent);
    ~OGLViewWidget() override;

    void PlotAtoms(std::vector<Vector3f> pos, std::vector<Vector3f> cols, View::Direction view_dir,
            float x_min,
            float x_max,
            float y_min,
            float y_max,
            float z_min,
            float z_max) {
        // TODO: centre on structure
        // TODO: get limits of view and show them

        MakeBuffers(pos, cols);

        SetCube(x_min, x_max, y_min, y_max, z_min, z_max);

        // TODO: might need to sort these vectors out
        auto v_d = directionEnumToVector(view_dir);
        Vector3f n_d = directionEnumToVector(View::Direction::Top);
        if (view_dir == View::Direction::Top || view_dir == View::Direction::Bottom)
            n_d = directionEnumToVector(View::Direction::Front);
        SetCamera(v_d*-1, v_d, n_d, ViewMode::Orthographic);
        // TODO: cube coords need to be defined for this to work
        fitView(1.0);
    }

    std::shared_ptr<OGLCamera> GetCamera() { return _camera;}

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

    OGLBillBoardTechnique _technique;

    std::shared_ptr<OGLCamera> _camera;

    std::vector<Vector3f> _cubeCoords;

    float _width, _height;

    Vector3f _background;

    QPoint _lastPos;

    void MakeBuffers(std::vector<Vector3f>& positions, std::vector<Vector3f>& colours)
    {
        makeCurrent();
        _technique.MakeBuffers(positions, colours);
        doneCurrent();
    }

    void SetCube(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);

    void SetCube(std::vector<Vector3f> Cube)
    {
        makeCurrent();
        _cubeCoords = std::move(Cube);
        doneCurrent();
    }

    void SetCamera(Vector3f position, Vector3f target, Vector3f up, float rx, float ry, float rz, ViewMode mode);
    void SetCamera(Vector3f position, Vector3f target, Vector3f up, ViewMode mode);

    void fitView(float extend = 1.0);
    void fitPerspView(float extend = 1.0);
    void fitOrthoView(float extend = 1.0);
};

#endif // MYGLWIDGET_H
