////
//// Created by jon on 02/06/18.
////

#include "borderlesswindow.h"
#ifdef _WIN32
#include <theme/thememanager.h>
#endif

BorderlessWindow::BorderlessWindow(QWidget *parent) :
        QMainWindow(parent)
{
    old_screen = nullptr;
}

#ifdef _WIN32
void BorderlessWindow::setMenuBar(QMenuBar *menuBar)
{
    QLayout *topLayout = layout();

    if (topLayout->menuBar() && topLayout->menuBar() != menuBar) {
        // Reparent corner widgets before we delete the old menu bar.
        QMenuBar *oldMenuBar = dynamic_cast<QMenuBar *>(topLayout->menuBar());
        if (menuBar) {
            // TopLeftCorner widget.
            QWidget *cornerWidget = oldMenuBar->cornerWidget(Qt::TopLeftCorner);
            if (cornerWidget)
                menuBar->setCornerWidget(cornerWidget, Qt::TopLeftCorner);
            // TopRightCorner widget.
            cornerWidget = oldMenuBar->cornerWidget(Qt::TopRightCorner);
            if (cornerWidget)
                menuBar->setCornerWidget(cornerWidget, Qt::TopRightCorner);
        }
        oldMenuBar->hide();
        oldMenuBar->deleteLater();
    }

    auto t_widget = new QWidget(this);

    auto t_layout = new QVBoxLayout(t_widget);
    t_layout->setSpacing(0);
    t_layout->setMargin(0);
    t_layout->setContentsMargins(0, 0, 0, 0);

    auto t_title = new FlatTitleBar(t_widget);
    t_title->setObjectName("title_bar");

    t_layout->addWidget(t_title);
    t_layout->addWidget(menuBar);

    t_widget->setLayout(t_layout);

    t_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    topLayout->setMenuBar(t_widget);

    setMenuBarVisible(ThemeManager::CurrentTheme != ThemeManager::Theme::Native);
}

void BorderlessWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    window_borderless();
}

void BorderlessWindow::window_borderless()
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

bool BorderlessWindow::testHitGlobal(QWidget* w, long x, long y)
{
    QPoint pg(x, y);
    QPoint p = w->mapFromGlobal(pg);
    bool hit = w->rect().contains(p);
    return hit;
}

void BorderlessWindow::window_shadow(int border)
{
    const MARGINS shadow = {border, border, border, border};
    DwmExtendFrameIntoClientArea((HWND)winId(), &shadow);
}

bool BorderlessWindow::nativeEvent(const QByteArray& eventType, void *message, long *result)
{
    bool is_borderless = ThemeManager::CurrentTheme != ThemeManager::Theme::Native;
    if (!is_borderless)
        return QWidget::nativeEvent(eventType, message, result);

    MSG* msg;
    if ( eventType == "windows_generic_MSG" )
        msg = reinterpret_cast<MSG*>(message);
    else
        return QWidget::nativeEvent(eventType, message, result);

    switch (msg->message)
    {
        case WM_NCCALCSIZE:
        {
            // https://stackoverflow.com/questions/24718872/problems-while-handling-the-wm-nccalcsize-message
            int cx = GetSystemMetrics(SM_CXSIZEFRAME);
            int cy = GetSystemMetrics(SM_CYSIZEFRAME);

            RECT *clientRect = &(reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam))->rgrc[0];

            int w = clientRect->right - clientRect->left;
            int h = clientRect->bottom - clientRect->top;

//            auto scr_rect = QApplication::desktop()->availableGeometry(this);
            auto scr_rect = QApplication::screenAt(rect().center())->availableGeometry();

            if (w > scr_rect.width() && h > scr_rect.height()) {
                clientRect->left += (cx);
                clientRect->top += (0);
                clientRect->right -= (cx);
                clientRect->bottom -= (cy);
            }

            *result = 0;
            return true;
        }
//        case WM_NCPAINT:
        case WM_NCHITTEST:
        {
            *result = 0;
            const LONG border_width = 8; //in pixels
            RECT winrect;
            GetWindowRect((HWND)winId(), &winrect);

            long x = GET_X_LPARAM(msg->lParam);
            long y = GET_Y_LPARAM(msg->lParam);

            bool resizeWidth = minimumWidth() != maximumWidth();
            bool resizeHeight = minimumHeight() != maximumHeight();

            if(resizeWidth)
            {
                //left border
                if (x >= winrect.left && x < winrect.left + border_width)
                {
                    *result = HTLEFT;
                }
                //right border
                if (x < winrect.right && x >= winrect.right - border_width)
                {
                    *result = HTRIGHT;
                }
            }
            if(resizeHeight)
            {
                //bottom border
                if (y < winrect.bottom && y >= winrect.bottom - border_width)
                {
                    *result = HTBOTTOM;
                }
                //top border
                if (y >= winrect.top && y < winrect.top + border_width)
                {
                    *result = HTTOP;
                }
            }
            if(resizeWidth && resizeHeight)
            {
                //bottom left corner
                if (x >= winrect.left && x < winrect.left + border_width &&
                    y < winrect.bottom && y >= winrect.bottom - border_width)
                {
                    *result = HTBOTTOMLEFT;
                }
                //bottom right corner
                if (x < winrect.right && x >= winrect.right - border_width &&
                    y < winrect.bottom && y >= winrect.bottom - border_width)
                {
                    *result = HTBOTTOMRIGHT;
                }
                //top left corner
                if (x >= winrect.left && x < winrect.left + border_width &&
                    y >= winrect.top && y < winrect.top + border_width)
                {
                    *result = HTTOPLEFT;
                }
                //top right corner
                if (x < winrect.right && x >= winrect.right - border_width &&
                    y >= winrect.top && y < winrect.top + border_width)
                {
                    *result = HTTOPRIGHT;
                }
            }

            // this handles if we are in the title bar, or the main content
            if(*result == 0) {
                // get the height of our title bar

                // I use the Qt version here, because it actually works with it's own fucky HiDPI stuff...
                auto qt_cursor_pos = QCursor::pos();
                auto* t_bar = menuWidget()->findChild<FlatTitleBar*>("title_bar");
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

void BorderlessWindow::setWindowTitle(const QString &title) {
    auto t_bar = menuWidget()->findChild<FlatTitleBar*>("title_bar");
    if (t_bar)
        t_bar->setTitle(title);

    QWidget::setWindowTitle(title);
}

void BorderlessWindow::changeEvent(QEvent *event) {
    // also compensate for maximised with extra padding
    if (event->type() == QEvent::WindowStateChange) {
        auto t_bar = menuWidget()->findChild<FlatTitleBar *>("title_bar");
        if (t_bar)
            t_bar->setMaximiseIcon();

        if (ThemeManager::CurrentTheme != ThemeManager::Theme::Native) {
            auto win = window();
            if (win->windowState().testFlag(Qt::WindowMaximized)) {
                int cy = GetSystemMetrics(SM_CYSIZEFRAME);
                win->setContentsMargins(0, cy, 0, 0);
            } else {
                win->setContentsMargins(0, 0, 0, 0);
            }
        }
    }
    else if (event->type() == QEvent::ActivationChange) {
        auto t_bar = menuWidget()->findChild<FlatTitleBar *>("title_bar");
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

void BorderlessWindow::setMenuBarVisible(bool visible) {
    auto* t_bar = menuWidget()->findChild<FlatTitleBar*>("title_bar");
    if(t_bar) {
        t_bar->setEnabled(visible);
        t_bar->setVisible(visible);
    }

    if (visible) {
        if (window()->windowState().testFlag(Qt::WindowMaximized)) {
            int cy = GetSystemMetrics(SM_CYSIZEFRAME);
            window()->setContentsMargins(0, cy, 0, 0);
        } else {
            window()->setContentsMargins(0, 0, 0, 0);
        }
    } else
        window()->setContentsMargins(0, 0, 0, 0);
}

#endif