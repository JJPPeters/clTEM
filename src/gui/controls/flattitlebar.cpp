//
// Created by jon on 02/06/18.
//

#include <QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include <QtCore/QFileInfo>
#include <QtWidgets/QFileIconProvider>
#include "flattitlebar.h"
#include <QScreen>
#include <iostream>
#include <QtWidgets/QStyleOption>
#include <QtGui/QPainter>

FlatTitleBar::FlatTitleBar(QWidget *parent, bool is_dialog) {
    auto app_icon = new QLabel();
    auto title = new QLabel("");
    auto btn_min = new QPushButton();
    auto btn_max = new QPushButton();
    auto btn_close = new QPushButton();

    btn_min->setObjectName("min");
    btn_max->setObjectName("max");
    btn_close->setObjectName("close");
    title->setObjectName("title");

    app_icon->setAccessibleName("app_icon");
    btn_min->setAccessibleName("title_min");
    btn_max->setAccessibleName("title_max");
    btn_close->setAccessibleName("title_close");
    title->setAccessibleName("title_title");

    auto *layout = new QHBoxLayout(this);
    layout->setSpacing(1);
    layout->setMargin(0);
    layout->setContentsMargins(10, 0, 0, 2);

    QFileInfo fileInfo(qApp->arguments().at(0));
    auto icon = QFileIconProvider().icon(fileInfo);
    auto icon_pxmp = icon.pixmap(18, 18);
    icon_pxmp.setDevicePixelRatio(devicePixelRatioF());
    app_icon->setPixmap(icon_pxmp);

    title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    title->setContentsMargins(6, 0, 0, 0);

    layout->addWidget(app_icon);
    layout->addWidget(title);
    if (!is_dialog) {
        layout->addWidget(btn_min);
        layout->addWidget(btn_max);
    }
    layout->addWidget(btn_close);

    layout->setStretch(0, 0);
    layout->setStretch(1, 1);

    setMaximiseIcon();

    connect(btn_min, &QPushButton::clicked, this, &FlatTitleBar::minimise_window);
    connect(btn_max, &QPushButton::clicked, this, &FlatTitleBar::maximise_window);
    connect(btn_close, &QPushButton::clicked, this, &FlatTitleBar::close_window);
}

bool FlatTitleBar::testHitButtonsGlobal(long x, long y) {

    QList<QPushButton*> lstBtns = findChildren<QPushButton*>();
    QPoint pg(x, y);

    for(const auto& btn : lstBtns) {
        QPoint p = btn->mapFromGlobal(pg);
        bool hit = btn->rect().contains(p);
        if (hit)
            return true;
    }

    return false;
}

bool FlatTitleBar::testHitButtonGlobal(QString name, long x, long y) {
    QPushButton* btn = findChild<QPushButton*>(name);
    QPoint pg(x, y);

    QPoint p = btn->mapFromGlobal(pg);
    return btn->rect().contains(p);
}

void FlatTitleBar::minimise_window() {
    auto win = window();

    win->showMinimized();
}

void FlatTitleBar::maximise_window() {
    auto win = window();

    if (win->windowState().testFlag(Qt::WindowMaximized))
        win->showNormal();
    else
        win->showMaximized();
}

void FlatTitleBar::close_window() {
    auto win = window();

    win->close();
}

void FlatTitleBar::setTitle(const QString &title) {
    auto t_widget = findChild<QLabel*>("title");

    if(t_widget)
        t_widget->setText(title);
}

void FlatTitleBar::setMaximiseIcon() {
    auto win = window();
    auto t_btn = findChild<QPushButton*>("max");

    if(t_btn) {
        if (win->windowState().testFlag(Qt::WindowMaximized)) {
            t_btn->setStyleSheet("image: url(:/Theme/icons/unmaximise.svg);");
        } else {
            t_btn->setStyleSheet("image: url(:/Theme/icons/maximise.svg);");
        }
    }
}
