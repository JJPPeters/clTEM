#include "tabpanel.h"

#include "tabpanelbar.h"

tabPanel::tabPanel(QWidget *parent) : QTabWidget(parent)
{
    tabPanelBar* temp = new tabPanelBar(this);

    setTabBar(temp);
}