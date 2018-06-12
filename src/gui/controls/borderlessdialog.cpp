////
//// Created by jon on 02/06/18.
////

#include <c++/7.3.0/iostream>
#include <theme/thememanager.h>
#include "borderlessdialog.h"

BorderlessDialog::BorderlessDialog(QWidget *parent) :
        QDialog(parent)
{}

void BorderlessDialog::addTitleBar()
{
    if(!layout())
        return;

    auto t_title = new FlatTitleBar(this, true);
    t_title->setObjectName("title_bar");

    t_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    layout()->setMenuBar(t_title);

    setMenuBarVisible(ThemeManager::CurrentTheme != ThemeManager::Theme::Native);
}

void BorderlessDialog::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
#ifdef Q_OS_WIN
//    if (ThemeManager::CurrentTheme != ThemeManager::Theme::Native)
        window_borderless();
#endif
}

#ifdef Q_OS_WIN
void BorderlessDialog::window_borderless()
{
    if (isVisible()) {
        int border = (int)(ThemeManager::CurrentTheme != ThemeManager::Theme::Native);
        window_shadow(border);
    }
}

bool BorderlessDialog::testHitGlobal(QWidget* w, long x, long y)
{
    QPoint pg(x, y);
    QPoint p = w->mapFromGlobal(pg);

    return w->rect().contains(p);
}

void BorderlessDialog::window_shadow(int border)
{
    const MARGINS shadow = {border, border, border, border};
    DwmExtendFrameIntoClientArea((HWND)winId(), &shadow);
}

bool BorderlessDialog::nativeEvent(const QByteArray& eventType, void *message, long *result)
{
    bool is_borderless = ThemeManager::CurrentTheme != ThemeManager::Theme::Native;
    auto* t_bar = dynamic_cast<FlatTitleBar*>(layout()->menuBar());

    if (!is_borderless) {
        return QWidget::nativeEvent(eventType, message, result);
    }

    MSG* msg;
    if ( eventType == "windows_generic_MSG" )
        msg = reinterpret_cast<MSG*>(message);
    else
        return QWidget::nativeEvent(eventType, message, result);

    switch (msg->message)
    {
        case WM_NCCALCSIZE:
        {
            *result = 0;
            return true;
        }
        case WM_NCHITTEST:
        {
            if (*result == HTCAPTION)
                return QWidget::nativeEvent(eventType, message, result);

            *result = 0;

            long x = GET_X_LPARAM(msg->lParam);
            long y = GET_Y_LPARAM(msg->lParam);

            // this handles if we are in the title bar, or the main content
            if(*result == 0) {
                    if (t_bar && testHitGlobal(t_bar, x, y) && !t_bar->testHitButtonsGlobal(x, y))
                        *result = HTCAPTION; // this says we are in a title bar...
                    else
                        *result = HTCLIENT; // this is client space
            }

            return true;
        } //end case WM_NCHITTEST
        case WM_CLOSE:
        {
            return close();
        }
        default: {
            return QWidget::nativeEvent(eventType, message, result);
        }
    }
}

void BorderlessDialog::setWindowTitle(const QString &title) {
    if(auto* t_bar = dynamic_cast<FlatTitleBar*>(layout()->menuBar()))
        t_bar->setTitle(title);

    QWidget::setWindowTitle(title);
}

void BorderlessDialog::changeEvent(QEvent *event) {
    // change the maximise icon if we need to

    // also compensate for maximised with extra padding
    if (event->type() == QEvent::WindowStateChange) {
        auto t_bar = layout()->menuBar()->findChild<FlatTitleBar *>("title_bar");
        if (t_bar)
            t_bar->setMaximiseIcon();

        auto win = window();
        if (win->windowState().testFlag(Qt::WindowMaximized)) {
            win->setContentsMargins(0, 9, 0, 0);
        } else {
            win->setContentsMargins(0, 0, 0, 0);
        }
    }

    QWidget::changeEvent(event);
}

#endif