#include "tabpanelbar.h"

tabPanelBar::tabPanelBar(QWidget *parent) : QTabBar(parent)
{
    preserve_height = false;
}

QSize tabPanelBar::sizeHint() const
{
    double h = 0.0;
    if (preserve_height)
        h = QTabBar::sizeHint().height();
    return QSize(0, h);
}

QSize tabPanelBar::minimumSizeHint() const
{
    double h = 0.0;
    if (preserve_height)
        h = QTabBar::sizeHint().height();
    return QSize(0, h);
}