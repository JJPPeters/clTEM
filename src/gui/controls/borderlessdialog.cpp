////
//// Created by jon on 02/06/18.
////

#include "borderlessdialog.h"
#ifdef _WIN32
#include <theme/thememanager.h>
#endif

BorderlessDialog::BorderlessDialog(QWidget *parent) :
        QDialog(parent)
{
    old_screen = nullptr;
}

#ifdef _WIN32
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
    window_borderless();

    auto prnt = dynamic_cast<QWidget*>(this->parent());
    if (prnt == nullptr)
        return;

    auto g_prnt = prnt->geometry();
    auto g_this = geometry();

    auto w = g_this.width();
    auto h = g_this.height();

    auto cntr_prnt = g_prnt.center();
    auto l = cntr_prnt.x() - w / 2;
    auto t = cntr_prnt.y() - h / 2;

    setGeometry(QRect(l, t, w, h));
}

void BorderlessDialog::window_borderless()
{
    if (isVisible()) {
        int border = (int)(ThemeManager::CurrentTheme != ThemeManager::Theme::Native);
        HWND id = (HWND)winId();
        SetWindowPos(id, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
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

//            long x = GET_X_LPARAM(msg->lParam);
//            long y = GET_Y_LPARAM(msg->lParam);

            // this handles if we are in the title bar, or the main content
            if(*result == 0) {
                // get the height of our title bar

                // I use the Qt version here, because it actually works with it's own fucky HiDPI stuff...
                auto qt_cursor_pos = QCursor::pos();
                auto* t_bar = dynamic_cast<FlatTitleBar*>(layout()->menuBar());
                if (t_bar && testHitGlobal(t_bar, qt_cursor_pos.x(), qt_cursor_pos.y()) && !t_bar->testHitButtonsGlobal(qt_cursor_pos.x(), qt_cursor_pos.y())) {
                    *result = HTCAPTION; // this says we are in a title bar...
                } else {
                    *result = HTCLIENT; // this is client space
                }
            }

            return true;
        } //end case WM_NCHITTEST
        case WM_CLOSE:
        {
            return close();
        }
    }

    return QWidget::nativeEvent(eventType, message, result);
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
        auto* t_bar = dynamic_cast<FlatTitleBar*>(layout()->menuBar());
        if (t_bar)
            t_bar->setMaximiseIcon();

        auto win = window();
        if (win->windowState().testFlag(Qt::WindowMaximized)) {
            win->setContentsMargins(0, 9, 0, 0);
        } else {
            win->setContentsMargins(0, 0, 0, 0);
        }
    } else if (event->type() == QEvent::ActivationChange) {
        auto* t_bar = dynamic_cast<FlatTitleBar*>(layout()->menuBar());
        if (this->isActiveWindow()) {
            t_bar->setStyleSheet("");
        } else {
            QColor disabled_col = qApp->palette().color(QPalette::Disabled, QPalette::Base);
            QColor disabled_tex_col = qApp->palette().color(QPalette::Disabled, QPalette::Text);
            t_bar->setStyleSheet("QLabel { color: " + disabled_tex_col.name() + ";} FlatTitleBar { background-color: " + disabled_col.name() + ";}");
        }
    }

    QWidget::changeEvent(event);
}

void BorderlessDialog::setMenuBarVisible(bool visible) {
    auto* t_bar = dynamic_cast<FlatTitleBar*>(layout()->menuBar());
    if(t_bar) {
        t_bar->setEnabled(visible);
        t_bar->setVisible(visible);
    }
}

#endif