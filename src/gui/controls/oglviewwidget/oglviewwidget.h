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

class OGLViewWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit OGLViewWidget(QWidget *parent = nullptr);
    ~OGLViewWidget() override;

    void MakeBuffers(std::vector<Vector3f>& positions, std::vector<Vector3f>& colours)
    {
        makeCurrent();
        _technique.MakeBuffers(positions, colours);
        doneCurrent();
    }

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

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    OGLBillBoardTechnique _technique;

    std::shared_ptr<OGLCamera> _camera;

    std::vector<Vector3f> _cubeCoords;

    float _width, _height;

    QPoint _lastPos;
};

#endif // MYGLWIDGET_H
