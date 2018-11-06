// myglwidget.cpp
#include "oglviewwidget.h"

#include <limits>

#include <iostream>

OGLViewWidget::OGLViewWidget(QWidget *parent) : QOpenGLWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::CustomContextMenu);

    _camera = nullptr;
    _rotation_offset = Vector3f(0.f, 0.f, 0.f);
    _drawRects = true;

    _width = width();
    _height = height();

    // use system colours
    auto bk_col = qApp->palette().brush(QPalette::Background).color();
    _background = Vector3f(bk_col.red(), bk_col.green(), bk_col.blue()) / 255.0f;

    _technique = std::make_shared<OGLBillBoardTechnique>();

    connect(this, &OGLViewWidget::customContextMenuRequested, this, &OGLViewWidget::contextMenuRequest);
}

bool OGLViewWidget::event(QEvent *event) {
    // this might get spammed a bit, not sure if it is supposed to
    if (event->type() == QEvent::PaletteChange) {
        auto bk_col = qApp->palette().brush(QPalette::Background).color();
        _background = Vector3f(bk_col.red(), bk_col.green(), bk_col.blue()) / 255.0f;
        repaint();
    }

    // very important or no other events will get through
    return QOpenGLWidget::event(event);
}

void OGLViewWidget::SetCamera(Vector3f position, Vector3f target, Vector3f up, Vector3f rot, ViewMode mode) {
    Vector3f origin(0.0f, 0.0f, 0.0f);

    _camera = std::make_shared<OGLCamera>(OGLCamera(position, target, up, origin, rot, _rotation_offset, mode));

    _camera->initOrthoProjection(10, -10, -10, 10, -100, 10000);

    _camera->SetWidthHeight(_width, _height);
    _camera->SetPixelRatio(devicePixelRatio());
}

void OGLViewWidget::SetCamera(Vector3f position, Vector3f target, Vector3f up, ViewMode mode) {
    SetCamera(position, target, up, Vector3f(0.f, 0.f, 0.f), mode);
}

void OGLViewWidget::fitView(float extend) {
    fitOrthoView(extend);
    update();
}

void OGLViewWidget::fitOrthoView(float extend) {
    float w = std::numeric_limits<float>::min();
    float h = std::numeric_limits<float>::min();

    // start by looping through our coords
    for (auto coord : _cubeCoords) {
        Vector4f coord4(coord, 1.0f);
        Vector4f MV_coord = _camera->getMV() * coord4;

        if (std::abs(MV_coord.x) > w) {
            w = std::abs(MV_coord.x);
        }
        if (std::abs(MV_coord.y) > h) {
            h = std::abs(MV_coord.y);
        }
    }

    float view_height, view_width;

    float aspect = _width / _height;

    if (w / h > aspect) // cube is wider than view
    {
        view_width = w * extend;
        view_height = view_width / aspect;
    } else {
        view_height = h * extend;
        view_width = view_height * aspect;
    }

    _camera->initOrthoProjection(view_height, -view_width, -view_height, view_width, -100, 10000);
}

void OGLViewWidget::initializeGL() {
    // without this get errors with "glVertexAttribPointer"
    auto _vao = new QOpenGLVertexArrayObject(this);
    _vao->create();
    _vao->bind(); // TODO: release this somewhere?

    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    QOpenGLExtraFunctions *glFuncsExtra = QOpenGLContext::currentContext()->extraFunctions();
    glFuncsExtra->initializeOpenGLFunctions();

    // this is the background colour...
    glClearColor(_background.x, _background.y, _background.z, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER); // this is odd, see reference in paintGL()

//    glEnable(GL_MULTISAMPLE);
//    glEnable(GL_SAMPLE_COVERAGE);
//    glEnable(GL_SAMPLE_SHADING);
//    glFuncsExtra->glMinSampleShading(1.0);

    // this is for alpha stuff
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    try {
        _technique->Init();
    } catch (std::runtime_error &err) {
        QMessageBox::critical(this, "OpenGL error", err.what(), QMessageBox::Ok);
        return;
    }
}

void OGLViewWidget::paintGL() {
    if (!_camera)
        return;

    // see for depth stuff
    // https://stackoverflow.com/questions/4189831/depth-test-inverted-by-default-in-opengl-or-did-i-get-it-wrong
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Matrix4f MV = _camera->getMV();
    Matrix4f P = _camera->getP();

    if (_technique)
        _technique->Render(MV, P, _camera->GetScreenSize());

    if (_drawRects) {
        for (auto &re : _recSlices) {
            glClear(GL_DEPTH_BUFFER_BIT);
            re->Render(MV, P, _camera->GetScreenSize());
        }
    }
    _camera->ResetViewPort();
}

void OGLViewWidget::resizeGL(int width, int height) {
    _width = width;
    _height = height;

    if (!_camera)
        return;

    _camera->SetWidthHeight(width, height);

    update();
}

void OGLViewWidget::mousePressEvent(QMouseEvent *event) {
    _lastPos = event->pos();
}

void OGLViewWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!_camera)
        return;

    // y swapped to avoid inverted axis
    int dx = event->x() - _lastPos.x();
    int dy = event->y() - _lastPos.y();

    auto test = QGuiApplication::queryKeyboardModifiers();

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

void OGLViewWidget::keyPressEvent(QKeyEvent *event) {
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

void OGLViewWidget::wheelEvent(QWheelEvent *event) {
    if (!_camera)
        return;

    if (event->delta() != 0) {
        _camera->OnScroll(event->delta());
        update();
    }
}

Vector3f OGLViewWidget::directionEnumToVector(View::Direction d) {
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

void OGLViewWidget::SetCube(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max) {

    std::vector<Vector3f> cc = {Vector3f(x_min, y_min, z_min),
                                Vector3f(x_max, y_min, z_min),
                                Vector3f(x_min, y_max, z_min),
                                Vector3f(x_max, y_max, z_min),
                                Vector3f(x_min, y_min, z_max),
                                Vector3f(x_max, y_min, z_max),
                                Vector3f(x_min, y_max, z_max),
                                Vector3f(x_max, y_max, z_max)};

    SetCube(cc);
}

void OGLViewWidget::SetCube(std::vector<Vector3f> Cube) {
    makeCurrent();
    _cubeCoords = std::move(Cube);
    doneCurrent();
}


void OGLViewWidget::PlotAtoms(std::vector<Vector3f> pos, std::vector<Vector3f> cols, View::Direction view_dir,
               float x_min,
               float x_max,
               float y_min,
               float y_max,
               float z_min,
               float z_max) {
    MakeScatterBuffers(pos, cols);

    auto x_offset = -(x_max + x_min) / 2;
    auto y_offset = -(y_max + y_min) / 2;
    auto z_offset = -(z_max + z_min) / 2;
    _rotation_offset = Vector3f(x_offset, y_offset, z_offset);

    SetCube(x_min, x_max, y_min, y_max, z_min, z_max);

    SetViewDirection(view_dir);
}

void OGLViewWidget::AddRectBuffer(float t, float l, float b, float r, float z, Vector4f &colour, OGL::Plane pl) {
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

void OGLViewWidget::SetViewDirection(View::Direction view_dir) {
    auto v_d = directionEnumToVector(view_dir);
    auto n_d = directionEnumToVector(View::Direction::Top);
    if (view_dir == View::Direction::Top || view_dir == View::Direction::Bottom)
        n_d = directionEnumToVector(View::Direction::Back);

    SetCamera(v_d * -1, v_d, n_d, ViewMode::Orthographic);

    // cube coords need to be defined for this to work
    fitView(1.0);
}

void OGLViewWidget::MakeScatterBuffers(std::vector<Vector3f> &positions, std::vector<Vector3f> &colours) {
    if (positions.size() != colours.size())
        throw std::runtime_error("OpenGL: Scatter position vector size does not match scatter colour vector size");

    makeCurrent();
    _technique->MakeBuffers(positions, colours);
    doneCurrent();
}

void OGLViewWidget::contextMenuRequest(QPoint pos) {
    if(QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier)
        return;

    auto menu = new QMenu(this);
    menu->addAction("Reset view", this, &OGLViewWidget::resetPressed);
    menu->popup(mapToGlobal(pos));
}