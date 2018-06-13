////
//// Created by jon on 02/06/18.
////
//
#ifndef CLTEM_BORDERLESSWINDOW_H
#define CLTEM_BORDERLESSWINDOW_H

#include <QtWidgets/QMainWindow>

#ifdef _WIN32
// https://forum.qt.io/topic/26108/customize-window-frame/9
#include <windowsx.h>
#include <dwmapi.h>
#include <gdiplus.h>

#include <windows.h>
#include <objidl.h>

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QApplication>
#include <QtWidgets/QLayout>
#include <QtWidgets/QMenuBar>

#include "flattitlebar.h"
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QDesktopWidget>

#endif

//// for the mouse move event filter stuff:
//// https://stackoverflow.com/questions/1935021/getting-mousemoveevents-in-qt

class BorderlessWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit BorderlessWindow(QWidget *parent = nullptr);

#ifdef _WIN32
    void setMenuBarVisible(bool visible);

    bool testHitGlobal(QWidget* w, long x, long y);

    void setWindowTitle(const QString& title);

    void changeEvent(QEvent* event) override;

#ifndef QT_NO_MENUBAR
    void setMenuBar(QMenuBar *menubar);
#endif

    void showEvent(QShowEvent *event) override;

    void window_borderless();

    void window_shadow(int border = -1);

    bool nativeEvent(const QByteArray& eventType, void *message, long *result) override;
#endif
};

#endif //CLTEM_BORDERLESSWINDOW_H
