//
// Created by jon on 02/06/18.
//

#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include "flattitlebar.h"

FlatTitleBar::FlatTitleBar(QWidget *parent) : m_parent(parent->window())
{
    auto title = new QLabel(parent->windowTitle());
    auto pPB = new QPushButton ("x");

    auto *layout = new QHBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(pPB);

    connect(pPB, &QPushButton::clicked, m_parent, &QWidget::close);
}

//void FlatTitleBar::mousePressEvent(QMouseEvent *event)
//{
//    // warning: for testing
//    // this doesnt work if the title bar is still there, it will skip a bit
//    if(event->button() == Qt::LeftButton)
//    {
//        m_pCursor = event->globalPos() - m_parent->geometry().topLeft();
//        event->accept();
//    }
//}
//
//void FlatTitleBar::mouseMoveEvent(QMouseEvent *event)
//{
//    if(event->buttons() & Qt::LeftButton)
//    {
//        m_parent->move(event->globalPos() - m_pCursor);
//        event->accept();
//    }
//}

#ifdef Q_OS_WIN
// https://forum.qt.io/topic/26108/customize-window-frame/9
//#include <WinUser.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <gdiplus.h>
//#include <GdiPlusColor.h>

#include <windows.h>
#include <objidl.h>


#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QApplication>

#include "flattitlebar.h"

#endif


//bool FlatTitleBar::nativeEvent(const QByteArray& eventType, void *message, long *result) {
//
//    MSG* msg;
//    if ( eventType == "windows_generic_MSG" )
//        msg = reinterpret_cast<MSG*>(message);
//    else
//        return QWidget::nativeEvent(eventType, message, result);
//
//    if (msg->message == WM_NCHITTEST) {
//        *result = HTCAPTION;
//        return true;
//    }
//
//}