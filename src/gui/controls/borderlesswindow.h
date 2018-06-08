//
// Created by jon on 02/06/18.
//

#ifndef CLTEM_BORDERLESSWINDOW_H
#define CLTEM_BORDERLESSWINDOW_H


//#include <QtWidgets/QMainWindow>
//#include <QApplication>
//#include <QMouseEvent>
// for testing
//#include <iostream>


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
#include <mainwindow.h>

#include "flattitlebar.h"

#endif

// for the mouse move event filter stuff:
// https://stackoverflow.com/questions/1935021/getting-mousemoveevents-in-qt

class BorderlessWindow : public MainWindow {
    Q_OBJECT

//    FlatTitleBar* tb;
public:
    explicit BorderlessWindow(QWidget *parent = 0);

    void showEvent(QShowEvent *event);

#ifdef Q_OS_WIN
    void window_borderless();

    void window_shadow();

//    bool winEvent(MSG *msg, long *result);
    bool nativeEvent(const QByteArray& eventType, void *message, long *result);
#endif

private:
//    // used to define where our edge is to grab it
//    static unsigned int border;
//
//    bool eventFilter(QObject *obj, QEvent *event) override;
//
//    void mouseEdgeEvent(QMouseEvent *event);
};


#endif //CLTEM_BORDERLESSWINDOW_H
