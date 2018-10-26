// myglwidget.cpp
#include "oglviewwidget.h"

#include <limits>

#include <iostream>

OGLViewWidget::OGLViewWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    _camera = nullptr;
    _rotation_offset = Vector3f(0.f, 0.f, 0.f);

    _width = width();
    _height = height();

    // use system colours
    // TODO: make this customisable
    auto bk_col = qApp->palette().brush(QPalette::Background).color();
    _background = Vector3f(bk_col.red(), bk_col.green(), bk_col.blue()) / 255.0f;

    _technique = std::make_shared<OGLBillBoardTechnique>();
}

OGLViewWidget::~OGLViewWidget()
{
}

void OGLViewWidget::SetCamera(Vector3f position, Vector3f target, Vector3f up, Vector3f rot, ViewMode mode)
{
    Vector3f origin(10.0f, 10.0f, 10.0f);

    _camera = std::make_shared<OGLCamera>(OGLCamera(position, target, up, origin, rot, _rotation_offset, mode));

    _camera->initPerspProjection(60, 1, 1, 1, 1000);
    _camera->initOrthoProjection(10, -10, -10, 10, -50, 1000);

    _camera->SetWidthHeight(_width, _height);
    _camera->SetPixelRatio(devicePixelRatio());
}

void OGLViewWidget::SetCamera(Vector3f position, Vector3f target, Vector3f up, ViewMode mode)
{
    SetCamera(position, target, up, Vector3f(0.f, 0.f, 0.f), mode);
}

void OGLViewWidget::fitView(float extend)
{
    if (_camera->getProjectionMode() == ViewMode::Perspective)
    {
        // this might want to be iterative as the angles change as the distance is changed
        // if this starts acting up, I would manually set the view to be in front of the cube here,
        // then run the function twice (make is use _camera->getCameraPosition()
        fitPerspView(extend);
    }
    else
    {
        fitOrthoView(extend);
    }

    update();
}

void OGLViewWidget::fitOrthoView(float extend)
{
    float w = std::numeric_limits<float>::min();
    float h = std::numeric_limits<float>::min();

    // start by looping through our coords
    for (auto coord : _cubeCoords)
    {
        Vector4f coord4(coord, 1.0f);
        Vector4f MV_coord = _camera->getMV() * coord4;

        if (std::abs(MV_coord.x) > w)
        {
            w = std::abs(MV_coord.x);
        }
        if (std::abs(MV_coord.y) > h)
        {
            h = std::abs(MV_coord.y);
        }
    }

    float view_height, view_width;

    float aspect = _width / _height;

    if (w/h > aspect) // cube is wider than view
    {
        view_width = w * extend;
        view_height = view_width / aspect;
    }
    else
    {
//        view_width = w * extend;
//        view_height = view_width / aspect;
        view_height = h * extend;
        view_width = view_height * aspect;
    }

    _camera->initOrthoProjection(view_height, -view_width, -view_height, view_width, -50, 1000);
}

void OGLViewWidget::fitPerspView(float extend)
{
    // WARNING:
    // This is all slow and shit, but remeber the bounding cube is only 8 coordinates
    // and this function is not called often. Therefore there is not much performace to
    // be gained.

    float max_ang = 0.0f;
    float max_z = 0.0f;
    float max_xy = 0.0f;

    float d = 1.0f;
    float fov = 60.0f;

    bool max_is_x = true;

    // should be OK setting to 0 as we are always centered
    float front = 0.0;

    std::vector<Vector4f> cube4(_cubeCoords.size());

    // loop through to get the front z position
    // might as well apply the model transform here too
    for (int i = 0; i < _cubeCoords.size(); ++i)
    {
        Vector4f coord4(_cubeCoords[i], 1.0f);
        cube4[i] = _camera->getM() * coord4;

        // less than as negatice is 'out of the screen' or close to us
        if (cube4[i].z < front)
            front = cube4[i].z;
    }

    // reposition the front so it is in front (not on same z plane to)
    front *= 2;

    // start by looping through our coords
    for (auto coord : cube4)
    {
        // here we get the distances from the camera to the corner coord
        // the camera x is the distance normal to the screen, which is z in opengl terms (hence the mismatch)
        float dz = coord.z - front;
        float dx = std::abs(coord.x);
        float dy = std::abs(coord.y);

        // angle can be negative or positive so we abs them
        float angle_x = std::abs(std::atan(dx / dz));
        float angle_y = std::abs(std::atan(dy / dz));

        // get the max angles
        // this should probably be more efficiently coded, but fuck it
        if (angle_y > max_ang)
        {
            max_ang = angle_y;
            max_z = coord.z;
            max_xy = std::abs(coord.y);
        }
        if (angle_x > max_ang)
        {
            max_ang = angle_x;
            max_z = coord.z;
            max_xy = std::abs(coord.x);
        }
    }

    float aspect = _width / _height;

    // TODO: check this isn't bullshit
    if (!max_is_x)
        fov = fov / aspect;

    // calculate the distance we need from the point with the highest angle
    // the max_z correction is because we calculate an offset from the corner, which is offset from the origin
    d = extend * max_xy / std::tan(ToRadian(fov / 2)) - max_z;

    // apply new position
    Vector3f position = _camera->getCameraPos();
    position.Normalize();
    position *= d;
    _camera->setCameraPos(position);

    // why init this?
    _camera->initPerspProjection(fov, _width, _height, 0.1, 1000.0f);
}

void OGLViewWidget::initializeGL()
{
    // without this get errors with "glVertexAttribPointer"
    auto _vao = new QOpenGLVertexArrayObject( this );
    _vao->create();
    _vao->bind(); // TODO: release this somewhere?

    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->initializeOpenGLFunctions();

    // this is the background colour...
    glClearColor(_background.x, _background.y, _background.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    //glEnable(GL_MULTISAMPLE);
    //glEnable(GL_SAMPLE_SHADING);

    // this is for alpha stuff
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    try {
        _technique->Init();
    } catch (std::runtime_error& err) {
        QMessageBox::critical(this, "OpenGL error", err.what(), QMessageBox::Ok);
        return;
    }
}

void OGLViewWidget::paintGL()
{
    if (!_camera)
        return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    Matrix4f MV = _camera->getMV();
    Matrix4f P = _camera->getP();

    if (_technique)
        _technique->Render(MV, P, _camera->GetScreenSize());

    for (auto &re : _recSlices) {
        glClear(GL_DEPTH_BUFFER_BIT);
        re->Render(MV, P, _camera->GetScreenSize());
    }
    _camera->ResetViewPort();
}

void OGLViewWidget::resizeGL(int width, int height)
{
    _width = width;
    _height = height;

    if (!_camera)
        return;

    _camera->SetWidthHeight(width, height);
//    _camera->SetPixelRatio(devicePixelRatio());

    update();
}

void OGLViewWidget::mousePressEvent(QMouseEvent *event)
{
    _lastPos = event->pos();
}

void OGLViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!_camera)
        return;

    // y swapped to avoid inverted axis
    int dx = event->x() - _lastPos.x();
    int dy = event->y() - _lastPos.y();

    // mouse right is pan, mouse left is rotate
    if (event->buttons() & Qt::LeftButton)
    {
        _camera->OnMouseRight(dx, dy);
        update();
    }
    else if (event->buttons() & Qt::RightButton)
    {
        _camera->OnMouseLeft(dx, dy);
        update();
    }

    _lastPos = event->pos();
}

void OGLViewWidget::keyPressEvent(QKeyEvent *event)
{
    if (!_camera)
        return;

    switch (event->key())
    {
    case Qt::Key_Up :
    {
        _camera->OnKeyboard(KeyPress::Up);
        update();
    }
    break;
    case Qt::Key_Down :
    {
       _camera->OnKeyboard(KeyPress::Down);
        update();
    }
    break;
    case Qt::Key_Left :
    {
        _camera->OnKeyboard(KeyPress::Left);
        update();
    }
    break;
    case Qt::Key_Right :
    {
        _camera->OnKeyboard(KeyPress::Right);
        update();
    }
    break;
    }
}

void OGLViewWidget::wheelEvent(QWheelEvent *event)
{
    if (!_camera)
        return;

    if (event->delta() != 0)
    {
        _camera->OnScroll(event->delta());
        update();
    }
}

Vector3f OGLViewWidget::directionEnumToVector(View::Direction d) {
    if (d == View::Direction::Front) {
        return Vector3f(100.0f, 0.0f, 0.0f);
    } else if (d == View::Direction::Back) {
        return Vector3f(-100.0f, 0.0f, 0.0f);
    } else if (d == View::Direction::Left) {
        return Vector3f(0.0f, 100.0f, 0.0f);
    } else if (d == View::Direction::Right) {
        return Vector3f(0.0f, -100.0f, 0.0f);
    } else if (d == View::Direction::Top) {
        return Vector3f(0.0f, 0.0f, -100.0f);
    } else {
        return Vector3f(0.0f, 0.0f, 100.0f);
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
