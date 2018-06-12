////
//// Created by jon on 02/06/18.
////

#include <c++/7.3.0/iostream>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include "borderlesswindow.h"
#include <QDesktopWidget>
#include <theme/thememanager.h>

BorderlessWindow::BorderlessWindow(QWidget *parent) :
        QMainWindow(parent)
{
}

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

    auto t_layout = new QVBoxLayout(this);
    t_layout->setSpacing(0);
    t_layout->setMargin(0);
    t_layout->setContentsMargins(0, 0, 0, 0);

    auto t_title = new FlatTitleBar(this);
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
#ifdef Q_OS_WIN
//    if (ThemeManager::CurrentTheme != ThemeManager::Theme::Native)
        window_borderless();
#endif
}

#ifdef Q_OS_WIN
void BorderlessWindow::window_borderless()
{
    if (isVisible()) {
        int border = (int)(ThemeManager::CurrentTheme != ThemeManager::Theme::Native);
        window_shadow(border);
    }
}

bool BorderlessWindow::testHitGlobal(QWidget* w, long x, long y)
{
    QPoint pg(x, y);
    QPoint p = w->mapFromGlobal(pg);

    return w->rect().contains(p);
}

void BorderlessWindow::window_shadow(int border)
{
    const MARGINS shadow = {border, border, border, border};
    DwmExtendFrameIntoClientArea((HWND)winId(), &shadow);
}

bool BorderlessWindow::nativeEvent(const QByteArray& eventType, void *message, long *result)
{
    bool is_borderless = ThemeManager::CurrentTheme != ThemeManager::Theme::Native;
    auto* t_bar = menuWidget()->findChild<FlatTitleBar*>("title_bar");

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
            // https://stackoverflow.com/questions/24718872/problems-while-handling-the-wm-nccalcsize-message
            int cx = GetSystemMetrics(SM_CXSIZEFRAME);
            int cy = GetSystemMetrics(SM_CYSIZEFRAME);

            RECT *clientRect = &(reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam))->rgrc[0];

            int w = clientRect->right - clientRect->left;
            int h = clientRect->bottom - clientRect->top;

            auto scr_rect = QApplication::desktop()->availableGeometry(this);

            if (w > scr_rect.width() && h > scr_rect.height()) {
                clientRect->left += (cx);
                clientRect->top += (0);
                clientRect->right -= (cx);
                clientRect->bottom -= (cy);
            }

            *result = 0;
            return true;
        }
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

    QWidget::changeEvent(event);
}

#endif