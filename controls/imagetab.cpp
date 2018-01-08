#include "imagetab.h"
#include "ui_imagetab.h"

ImageTab::ImageTab(QWidget *parent, std::string name, TabType t) :
    QWidget(parent), TabName(name), MyType(t),
    ui(new Ui::ImageTab)
{
    ui->setupUi(this);
    ui->widget;
}

ImageTab::~ImageTab()
{
    delete ui;
}

ImagePlotWidget *ImageTab::getPlot()
{
    return ui->widget;
}
