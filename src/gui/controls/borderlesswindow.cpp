//
// Created by jon on 02/06/18.
//

#include <QtCore/QEvent>
#include <QtWidgets/QVBoxLayout>

#include "borderlesswindow.h"

#include "ui_mainwindow.h"

//unsigned int BorderlessWindow::border = 5;

BorderlessWindow::BorderlessWindow(QWidget *parent) :
        MainWindow(parent)
{
    ui->titleBar = new FlatTitleBar(this);
    ui->mainLayout->insertWidget(1, ui->menuBar);

//    ui->mainLayout->setStretch(0,0);
//    ui->mainLayout->setStretch(1,0);
//    ui->mainLayout->setStretch(2,0);
//    ui->mainLayout->setStretch(3,0);
//    ui->mainLayout->setStretch(4,1);
}

void BorderlessWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
#ifdef Q_OS_WIN
    window_borderless();
#endif
}

#ifdef Q_OS_WIN
void BorderlessWindow::window_borderless()
{
    if (isVisible())
    {
        //defaultStyle = (WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME)
        SetWindowLongPtr((HWND)winId(), GWL_STYLE, WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX );

        window_shadow();

        SetWindowPos((HWND)winId(), 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
    }
}

void BorderlessWindow::window_shadow()
{
    const MARGINS shadow = { 1, 1, 1, 1 };
//    DwmExtendFrameIntoClientArea((HWND)winId(), &shadow);
}

bool BorderlessWindow::nativeEvent(const QByteArray& eventType, void *message, long *result)
{
    MSG* msg;
    if ( eventType == "windows_generic_MSG" )
        msg = reinterpret_cast<MSG*>(message);
    else
        return QWidget::nativeEvent(eventType, message, result);

    switch (msg->message)
    {
        case WM_NCCALCSIZE:
        {
            //this kills the window frame and title bar we added with
            //WS_THICKFRAME and WS_CAPTION
            *result = 0;
            return true;
        }
        case WM_NCHITTEST:
        {
            if (*result == HTCAPTION)
                return QWidget::nativeEvent(eventType, message, result);

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

//            //TODO: allow move?
            if(*result == 0) {
                if (y < winrect.top + 30 && x < winrect.right - 50) {
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
        default:
            return QWidget::nativeEvent(eventType, message, result);
    }
}

#endif