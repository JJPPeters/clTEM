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

class OGLViewWidget : public QOpenGLWidget {
Q_OBJECT

signals:

    void resetView();

public:
    explicit OGLViewWidget(QWidget *parent);

    ~OGLViewWidget() override = default;

    bool event(QEvent *event) override;

    void setDrawRects(bool v) { _drawRects = v; }

    bool getDrawRects() { return _drawRects; }

    void clearRectBuffers() { _recSlices.clear(); }

    void PlotAtoms(std::vector<Vector3f> pos, std::vector<Vector3f> cols, View::Direction view_dir,
                   float x_min,
                   float x_max,
                   float y_min,
                   float y_max,
                   float z_min,
                   float z_max);

    void AddRectBuffer(float t, float l, float b, float r, float z, Vector4f &colour, OGL::Plane pl);

    void SetViewDirection(View::Direction view_dir);

    std::shared_ptr<OGLCamera> GetCamera() { return _camera; }

    void SetCube(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);

    void SetCube(std::vector<Vector3f> Cube);

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

    void MakeScatterBuffers(std::vector<Vector3f> &positions, std::vector<Vector3f> &colours);

    void SetCamera(Vector3f position, Vector3f target, Vector3f up, Vector3f rot, ViewMode mode);

    void SetCamera(Vector3f position, Vector3f target, Vector3f up, ViewMode mode);

    void fitOrthoView(float extend = 1.0);

    void contextMenuRequest(QPoint pos);

private slots:

    void resetPressed() { emit resetView(); }
};

#endif // MYGLWIDGET_H
