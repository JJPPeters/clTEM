#include "imagetab.h"
#include "ui_imagetab.h"

ImageTab::ImageTab(QWidget *parent, std::string name, TabType t) :
    QWidget(parent), TabName(name), MyType(t),
    ui(new Ui::ImageTab)
{
    ui->setupUi(this);
    ui->widget;

    connect(ui->widget, &ImagePlotWidget::saveDataClicked, this, &ImageTab::forwardSaveData);
    connect(ui->widget, &ImagePlotWidget::saveImageClicked, this, &ImageTab::forwardSaveImage);
}

ImageTab::~ImageTab()
{
    delete ui;
}

ImagePlotWidget *ImageTab::getPlot()
{
    return ui->widget;
}

nlohmann::json ImageTab::getSettings() {
    auto j = settings;
    j["ctem"]["cropped padding"] = ui->widget->getCropImage();
    return j;
}
