////
//// Created by jon on 02/06/18.
////
//
#ifndef CLTEM_BORDERLESSWINDOW_H
#define CLTEM_BORDERLESSWINDOW_H

#include <QtWidgets/QMainWindow>

#ifdef Q_OS_WIN
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
#include "borderlesswindow.h"

#endif

//// for the mouse move event filter stuff:
//// https://stackoverflow.com/questions/1935021/getting-mousemoveevents-in-qt

class BorderlessWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit BorderlessWindow(QWidget *parent = nullptr);

    void setMenuBarVisible(bool visible) {
        auto* t_bar = menuWidget()->findChild<FlatTitleBar*>("title_bar");
        if(t_bar) {
            t_bar->setEnabled(visible);
            t_bar->setVisible(visible);
        }
    }

    bool testHitGlobal(QWidget* w, long x, long y);

    void setWindowTitle(const QString& title);

    void changeEvent(QEvent* event);

#ifndef QT_NO_MENUBAR
    void setMenuBar(QMenuBar *menubar);
#endif

    void showEvent(QShowEvent *event) override;

#ifdef Q_OS_WIN
    void window_borderless();

    void window_shadow();

    bool nativeEvent(const QByteArray& eventType, void *message, long *result) override;
#endif
};

#endif //CLTEM_BORDERLESSWINDOW_H
