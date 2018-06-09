//
// Created by jon on 02/06/18.
//

#include <QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include "flattitlebar.h"

FlatTitleBar::FlatTitleBar(QWidget *parent)
{
    auto title = new QLabel(parent->windowTitle());
    auto btn_min = new QPushButton ("_");
    auto btn_max = new QPushButton ("□");
    auto btn_close = new QPushButton ("x");

    btn_min->setObjectName("min");
    btn_max->setObjectName("max");
    btn_close->setObjectName("close");

    auto *layout = new QHBoxLayout(this);
    layout->setSpacing(1);
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 2);

    layout->addWidget(title);
    layout->addWidget(btn_min);
    layout->addWidget(btn_max);
    layout->addWidget(btn_close);

    int h = 30;
    int w = (int) (h*1.5);
    btn_min->setFixedSize(w, h);
    btn_max->setFixedSize(w, h);
    btn_close->setFixedSize(w, h);

    connect(btn_min, &QPushButton::clicked, this, &FlatTitleBar::minimise_window);
    connect(btn_max, &QPushButton::clicked, this, &FlatTitleBar::maximise_window);
    connect(btn_close, &QPushButton::clicked, qApp, &QApplication::quit);
}

#include <iostream>
#include <QtWidgets/QStyleOption>
#include <QtGui/QPainter>

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
