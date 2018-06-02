//
// Created by jon on 02/06/18.
//

#include <QtCore/QEvent>

#include "borderlesswindow.h"

unsigned int BorderlessWindow::border = 5;

BorderlessWindow::BorderlessWindow(QWidget *parent) :
        QMainWindow(parent)
{
    qApp->installEventFilter(this);
}

bool BorderlessWindow::eventFilter(QObject *obj, QEvent *event)
{
//    if (qobject_cast<BorderlessWindow*>(obj) == NULL)
//        return false;

    if (event->type() == QEvent::MouseMove) {
        auto *me = dynamic_cast<QMouseEvent*>(event);
        mouseEdgeEvent(me);
    }
    return false;
}

void BorderlessWindow::mouseEdgeEvent(QMouseEvent *event) {
    auto pos = event->windowPos();

    bool l = pos.x() < border;
    bool r = size().width() - pos.x() < border;
    bool t = pos.y() < border;
    bool b = size().height() - pos.y() < border;

    if (!(t || l || b || r)) { // we are not at any border
        setCursor(Qt::ArrowCursor);
        return;
    }

    if (t && l) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (t && r) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (b && l) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (b && r) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (t) {
        setCursor(Qt::SizeVerCursor);
    } else if (l) {
        setCursor(Qt::SizeHorCursor);
    } else if (b) {
        setCursor(Qt::SizeVerCursor);
    } else if (r) {
        setCursor(Qt::SizeHorCursor);
    }

}
