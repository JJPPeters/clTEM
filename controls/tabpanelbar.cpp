#include "tabpanelbar.h"

tabPanelBar::tabPanelBar(QWidget *parent) : QTabBar(parent)
{

}

QSize tabPanelBar::sizeHint() const
{
    return QSize(0, 0);
}

QSize tabPanelBar::minimumSizeHint() const
{
    return QSize(0, 0);
}