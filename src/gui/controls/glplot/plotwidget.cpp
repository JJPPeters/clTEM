// myglwidget.cpp
#include "plotwidget.h"

#include <limits>

#include <iostream>
#include <techniques/scattertechnique.h>

namespace PGL {
    PlotWidget::PlotWidget(QWidget *parent, int msaa) : QOpenGLWidget(parent) {
        setFocusPolicy(Qt::StrongFocus);
        setContextMenuPolicy(Qt::CustomContextMenu);

        _camera = nullptr;
        _framebuffer = nullptr;

        _msaa = msaa;

        _width = width();
        _height = height();

        // use system colours
        auto bk_col = qApp->palette().brush(QPalette::Background).color();
        _background = Vector3f(bk_col.red(), bk_col.green(), bk_col.blue()) / 255.0f;

        connect(this, &PlotWidget::customContextMenuRequested, this, &PlotWidget::contextMenuRequest);
    }

    PlotWidget::~PlotWidget() {
        makeCurrent();

        _framebuffer.reset();

        _techniques.clear();

        doneCurrent();
    }

    bool PlotWidget::event(QEvent *event) {
        // this might get spammed a bit, not sure if it is supposed to
        if (event->type() == QEvent::PaletteChange) {
            auto bk_col = qApp->palette().brush(QPalette::Background).color();
            _background = Vector3f(bk_col.red(), bk_col.green(), bk_col.blue()) / 255.0f;
            repaint();
        }

        // very important or no other events will get through
        return QOpenGLWidget::event(event);
    }

    void PlotWidget::SetCamera(Vector3f position, Vector3f target, Vector3f up, Vector3f rot, ViewMode mode) {
        Vector3f origin(0.0f, 0.0f, 0.0f);
        Vector3f rotation_offset(0.0f, 0.0f, 0.0f);

        _camera = std::make_shared<PGL::Camera>(position, target, up, origin, rot, rotation_offset, mode);

        _camera->setOrthoProjection(10, -10, -10, 10, -100, 10000);

        _camera->setWidthHeight(_width, _height);
        _camera->setPixelRatio(devicePixelRatio());
    }

    void PlotWidget::SetCamera(Vector3f position, Vector3f target, Vector3f up, ViewMode mode) {
        SetCamera(position, target, up, Vector3f(0.f, 0.f, 0.f), mode);
    }

    void PlotWidget::initializeGL() {
        try {
            // I don't quite understand why we need this, but
            // without this get errors with "glVertexAttribPointer"
            // TODO: use non Qt version of this (for slightly easier portability later?
            auto _vao = new QOpenGLVertexArrayObject(this);
            _vao->create();
            _vao->bind(); // TODO: Do I need to release this somewhere?

            QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
            glFuncs->initializeOpenGLFunctions();

            QOpenGLExtraFunctions *glFuncsExtra = QOpenGLContext::currentContext()->extraFunctions();
            glFuncsExtra->initializeOpenGLFunctions();

            // this is the background colour...
            // see for depth stuff
            // https://stackoverflow.com/questions/4189831/depth-test-inverted-by-default-in-opengl-or-did-i-get-it-wrong
            glClearColor(_background.x, _background.y, _background.z, 1.0f);
            glClearDepth(0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_MULTISAMPLE);
            glEnable(GL_SAMPLE_COVERAGE);
            glEnable(GL_SAMPLE_SHADING);
            glFuncsExtra->glMinSampleShading(1.0);

            // this is for alpha stuff
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_GEQUAL); // this is odd, see reference in paintGL()

            _framebuffer = std::make_shared<PGL::Framebuffer>(_width, _height, 1.0, _msaa);
            //_camera is created in the set viewdirection method!
            SetViewDirection(View::Top);

            for (auto &technique: _techniques)
                technique->Init();

        } catch (std::exception &e) {
            emit initError(e.what());
        }

    }

    void PlotWidget::paintGL() {

        QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
        glFuncs->initializeOpenGLFunctions();

        GLint def_framebuffer_signed = 0;
        glFuncs->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &def_framebuffer_signed);
        GLuint def_framebuffer = static_cast<GLuint>(def_framebuffer_signed);

        _framebuffer->Bind();

        glClearColor(_background.x, _background.y, _background.z, 1.0f);
        glFuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Do some camera crap
        Matrix4f MV = _camera->getMV();
        Matrix4f P = _camera->getP();

        Vector2f screen_size(_width, _height);

        // TODO: handle order of objects (or will 3d-ness handle this nicely?)

        for (auto &technique: _techniques)
            //technique->Render(MV, P, screen_size);
            if (technique->getVisible())
                technique->Render(MV, P, _camera->getPixelSize());

        _framebuffer->Blit(def_framebuffer);
        _framebuffer->Unbind();
    }

    void PlotWidget::resizeGL(int width, int height) {
        _width = width;
        _height = height;

        // if (_camera) // TODO: is this check needed really??
        _camera->setWidthHeight(width, height);

        _framebuffer->Resize(_width, _height, _camera->getPixelRatio());
    }

    Eigen::Matrix<float, 3, 2> PlotWidget::GetSceneLimits() {
        // Using rows as x, y, z and columns and min, max
        Eigen::Matrix<float, 3, 2> limits;
        limits << std::numeric_limits<float>::max(), std::numeric_limits<float>::min(),
                std::numeric_limits<float>::max(), std::numeric_limits<float>::min(),
                std::numeric_limits<float>::max(), std::numeric_limits<float>::min();

        for (auto &technique: _techniques) {
            // TODO: is there a nice way to do this in Eigen?
            auto tl = technique->GetLimits();
            // x
            if (tl(0, 0) < limits(0, 0))
                limits(0, 0) = tl(0, 0);
            if (tl(0, 1) > limits(0, 1))
                limits(0, 1) = tl(0, 1);
            // y
            if (tl(1, 0) < limits(1, 0))
                limits(1, 0) = tl(1, 0);
            if (tl(1, 1) > limits(1, 1))
                limits(1, 1) = tl(1, 1);
            // z
            if (tl(2, 0) < limits(2, 0))
                limits(2, 0) = tl(2, 0);
            if (tl(2, 1) > limits(2, 1))
                limits(2, 1) = tl(2, 1);
        }

        return limits;
    }

    std::vector<Vector3f> PlotWidget::GetBoundingCube() {
        Eigen::Matrix<float, 3, 2> limits = GetSceneLimits();
        std::vector<Vector3f> bounding_cube(8);

        bounding_cube[0] = Vector3f(limits(0,0), limits(1,0), limits(2,0));
        bounding_cube[1] = Vector3f(limits(0,1), limits(1,0), limits(2,0));
        bounding_cube[2] = Vector3f(limits(0,1), limits(1,1), limits(2,0));
        bounding_cube[3] = Vector3f(limits(0,0), limits(1,1), limits(2,0));

        bounding_cube[4] = Vector3f(limits(0,0), limits(1,0), limits(2,1));
        bounding_cube[5] = Vector3f(limits(0,1), limits(1,0), limits(2,1));
        bounding_cube[6] = Vector3f(limits(0,1), limits(1,1), limits(2,1));
        bounding_cube[7] = Vector3f(limits(0,0), limits(1,1), limits(2,1));

        return bounding_cube;
    }

    void PlotWidget::FitView(float extend) {
        FitOrthoView(extend);
        update();
    }

    void PlotWidget::FitOrthoView(float extend) {

        auto bounding_cube = GetBoundingCube();

        float min_x = std::numeric_limits<float>::max();
        float min_y = std::numeric_limits<float>::max();
        float max_x = std::numeric_limits<float>::min();
        float max_y = std::numeric_limits<float>::min();

        auto MV = _camera->getMV();

        for (auto coord : bounding_cube) {
            Vector4f coord4(coord, 1.0f);
            Vector4f MV_coord = MV * coord4;

            if (MV_coord.x < min_x)
                min_x = MV_coord.x;
            if (MV_coord.y < min_y)
                min_y = MV_coord.y;

            if (MV_coord.x > max_x)
                max_x = MV_coord.x;
            if (MV_coord.y > max_y)
                max_y = MV_coord.y;
        }

        float w = max_x - min_x;
        float h = max_y - min_y;

        float aspect = _width / _height;

        float view_width, view_height;

        if (w / h > aspect) {
            view_width = w * extend;
            view_height = view_width / aspect;
        } else {
            view_height = h * extend;
            view_width = view_height * aspect;
        }

        float mid_x = (min_x + max_x) / 2.0f;
        float mid_y = (min_y + max_y) / 2.0f;

        float t = mid_y + view_height / 2.0f;
        float l = mid_x - view_width / 2.0f;
        float b = mid_y - view_height / 2.0f;
        float r = mid_x + view_width / 2.0f;

        _camera->setOrthoProjection(t, l, b,r);
    }

    void PlotWidget::mousePressEvent(QMouseEvent *event) {
        _lastPos = event->pos();
    }

    void PlotWidget::mouseMoveEvent(QMouseEvent *event) {
        if (!_camera)
            return;

        // y swapped to avoid inverted axis
        int dx = event->x() - _lastPos.x();
        int dy = event->y() - _lastPos.y();

        //auto test = QGuiApplication::queryKeyboardModifiers();

        // mouse right is pan, mouse left is rotate
        if (event->buttons() & Qt::LeftButton) {
            _camera->OnMousePan(dx, dy);
            update();
        } else if (event->buttons() & Qt::RightButton && QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) {
            _camera->OnMouseRotate(dx, dy);
            update();
        }

        _lastPos = event->pos();
    }

    void PlotWidget::keyPressEvent(QKeyEvent *event) {
        if (!_camera)
            return;

        switch (event->key()) {
            case Qt::Key_Up : {
                _camera->OnKeyboardNudge(KeyPress::Up);
                update();
            }
                break;
            case Qt::Key_Down : {
                _camera->OnKeyboardNudge(KeyPress::Down);
                update();
            }
                break;
            case Qt::Key_Left : {
                _camera->OnKeyboardNudge(KeyPress::Left);
                update();
            }
                break;
            case Qt::Key_Right : {
                _camera->OnKeyboardNudge(KeyPress::Right);
                update();
            }
                break;
            default:break;
        }
    }

    void PlotWidget::wheelEvent(QWheelEvent *event) {
        if (!_camera)
            return;

        if (event->delta() != 0) {
            float px = event->pos().x() / _width;
            float py = 1.0f - event->pos().y() / _height;

            _camera->OnScroll(event->delta(), px, py);
            update();
        }
    }

    Vector3f PlotWidget::directionEnumToVector(View::Direction d) {
        if (d == View::Direction::Front) {
            return Vector3f(0.0f, -1000.0f, 0.0f);
        } else if (d == View::Direction::Back) {
            return Vector3f(0.0f, 1000.0f, 0.0f);
        } else if (d == View::Direction::Left) {
            return Vector3f(-1000.0f, 0.0f, 0.0f);
        } else if (d == View::Direction::Right) {
            return Vector3f(1000.0f, 0.0f, 0.0f);
        } else if (d == View::Direction::Top) {
            return Vector3f(0.0f, 0.0f, 1000.0f);
        } else {
            return Vector3f(0.0f, 0.0f, -1000.0f);
        }
    }

    void PlotWidget::SetViewDirection(View::Direction view_dir) {
        auto v_d = directionEnumToVector(view_dir);
        auto n_d = directionEnumToVector(View::Direction::Top);
        if (view_dir == View::Direction::Top || view_dir == View::Direction::Bottom)
            n_d = directionEnumToVector(View::Direction::Back);

        SetCamera(v_d * -1, v_d, n_d, ViewMode::Orthographic);
    }

    void PlotWidget::contextMenuRequest(QPoint pos) {
        if(QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier)
            return;

        auto menu = new QMenu(this);
        menu->addAction("Reset view", this, &PlotWidget::resetPressed);
        menu->popup(mapToGlobal(pos));
    }

    std::weak_ptr<PGL::Scatter> PlotWidget::scatter(std::vector<Vector3f> positions, std::vector<Vector3f> colours) {
        makeCurrent();

        auto plot_item = std::make_shared<PGL::Scatter>(positions, colours);

        addItem(std::dynamic_pointer_cast<PGL::Technique>(plot_item));

        doneCurrent();

        return plot_item;
    }

    std::weak_ptr<PGL::Rectangle> PlotWidget::rectangle(float t, float l, float b, float r, float z, Vector4f &colour, PGL::Plane pl) {
        makeCurrent();

        auto plot_item = std::make_shared<PGL::Rectangle>(t, l, b, r, z, colour, pl);

        addItem(std::dynamic_pointer_cast<PGL::Technique>(plot_item));

        doneCurrent();

        return plot_item;
    }
}