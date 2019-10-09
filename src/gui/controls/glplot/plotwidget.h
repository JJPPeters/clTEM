#include <utility>

#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QWidget>
#include <QtWidgets>
#include <QOpenGLWidget>
#include <QtOpenGL>

#include <memory>

#include "GL/glu.h"

#include "arraybuffer.h"
#include "techniques/technique.h"
//#include "scattertechnique.h"
#include "camera.h"
//#include "oglrectangletechnique.h"

#include <Eigen/Dense>

#include "framebuffer.h"

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

namespace PGL {
    class PlotWidget : public QOpenGLWidget {
        Q_OBJECT

    signals:
        void resetView();

//        void initError(std::string);

    public:
        explicit PlotWidget(QWidget *parent);

        Eigen::Matrix<float, 3, 2> GetSceneLimits();

        void FitView(float extend = 1.1);

        void SetViewDirection(View::Direction view_dir);

        void addItem(std::shared_ptr<PGL::Technique> technique) {
            auto position = std::find(_techniques.begin(), _techniques.end(), technique);
            if (position == _techniques.end()) // i.e. element does not already exist
                _techniques.push_back(technique);
        }

        void removeItem(std::shared_ptr<PGL::Technique> technique) {
            auto position = std::find(_techniques.begin(), _techniques.end(), technique);
            if (position != _techniques.end())
                _techniques.erase(position);
        }

//    void SetViewDirection(View::Direction view_dir);

//    std::shared_ptr<OGLCamera> GetCamera() { return _camera; }

    protected:
        bool event(QEvent *event) override;

        void initializeGL() override;

        void paintGL() override;

        void resizeGL(int width, int height) override;

        void mousePressEvent(QMouseEvent *event) override;

        void mouseMoveEvent(QMouseEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

        void keyPressEvent(QKeyEvent *event) override;

    private:
        void FitOrthoView(float extend = 1.0);

        std::vector<std::shared_ptr<PGL::Technique>> _techniques;

        std::shared_ptr<PGL::Framebuffer> _framebuffer;

        Vector3f directionEnumToVector(View::Direction d);

//    std::shared_ptr<OGLBillBoardTechnique> _technique;
//    std::vector<std::shared_ptr<OGLRectangleTechnique>> _recSlices;

        std::shared_ptr<PGL::Camera> _camera;

//    std::vector<Vector3f> _cubeCoords;

        // width of the openGL window
        float _width, _height;

        // Structure limits
        // Vector3f _rotation_offset;

        Vector3f _background;

        QPoint _lastPos;

//    void MakeScatterBuffers(std::vector<Vector3f> &positions, std::vector<Vector3f> &colours);

        void SetCamera(Vector3f position, Vector3f target, Vector3f up, Vector3f rot, ViewMode mode);

        void SetCamera(Vector3f position, Vector3f target, Vector3f up, ViewMode mode);

        void contextMenuRequest(QPoint pos);

    private slots:
        void resetPressed() { emit resetView(); }
    };
}

#endif // MYGLWIDGET_H
