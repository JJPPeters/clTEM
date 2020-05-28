////
//// Created by jon on 02/06/18.
////
//
#ifndef CLTEM_BORDERLESSDIALOG_H
#define CLTEM_BORDERLESSDIALOG_H

#include <QtWidgets/QDialog>

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
#include "borderlesswindow.h"

#endif

//// for the mouse move event filter stuff:
//// https://stackoverflow.com/questions/1935021/getting-mousemoveevents-in-qt

class BorderlessDialog : public QDialog {
    Q_OBJECT

public:
    explicit BorderlessDialog(QWidget *parent = nullptr);

#ifdef _WIN32

    void setMenuBarVisible(bool visible);

    bool testHitGlobal(QWidget* w, long x, long y);

    void setWindowTitle(const QString& title);

    void changeEvent(QEvent* event) override;

#ifndef QT_NO_MENUBAR
    void addTitleBar();
#endif

    void showEvent(QShowEvent *event) override;

    void window_borderless();

    void window_shadow(int border = -1);

    bool nativeEvent(const QByteArray& eventType, void *message, long *result) override;

private:
    QScreen* old_screen;

    // This is to fix an error when moving the borderless window between screens
    // basically I detect when I have moved between screens, and just set the size to be the same!
    void moveEvent(QMoveEvent * e) override
    {
        if (old_screen == nullptr) {
            old_screen = screen();
            qDebug() << "dialog new screen";

//            HWND id = (HWND)winId();
//            SetWindowPos(id, NULL, 0, 0, 0, 0,
//                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
//                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
//            update();
        } else if (old_screen == screen()) {
            // screen is the same, we don't case...
        } else {
            qDebug() << "dialog change screen";
            old_screen = screen();

            SetWindowPos((HWND) winId(), NULL, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_FRAMECHANGED | SWP_NOACTIVATE);
        }
    }
#endif
};

#endif //CLTEM_BORDERLESSDIALOG_H
