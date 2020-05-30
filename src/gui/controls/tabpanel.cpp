#include "tabpanel.h"

tabPanel::tabPanel(QWidget *parent) : QTabWidget(parent)
{
    auto* temp = new tabPanelBar(this);

    setTabBar(temp);
}