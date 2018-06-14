#include <utils/stringutils.h>
#include "imagetab.h"
#include "ui_imagetab.h"

ImageTab::ImageTab(QWidget *parent, std::string name, TabType t, bool is_complex) :
    QWidget(parent), TabName(name), MyType(t),
    ui(new Ui::ImageTab)
{
    ui->setupUi(this);
    ui->widget;

    if (!is_complex)
        ui->cmbComplex->hide();

    connect(ui->widget, &ImagePlotWidget::saveDataClicked, this, &ImageTab::forwardSaveData);
    connect(ui->widget, &ImagePlotWidget::saveImageClicked, this, &ImageTab::forwardSaveImage);

    connect(ui->widget, &ImagePlotWidget::mouseHoverEvent, this, &ImageTab::updatePositionLabels);
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

void ImageTab::updatePositionLabels(double x, double y) {
    QString xs = "x: " + Utils_Qt::numToQString(x) + " " + image_units;
    QString ys = "y: " + Utils_Qt::numToQString(y) + " " + image_units;

    ui->lblX->setText(xs);
    ui->lblY->setText(ys);
}

ShowComplex ImageTab::getComplexDisplayOption() {

    auto t = ui->cmbComplex->currentText();
    if (t == "Amplitude")
        return ShowComplex::Amplitude;
    else if (t == "Phase")
        return ShowComplex::Phase;
    else if (t == "Real")
        return ShowComplex::Real;
    else if (t == "Imaginary")
        return ShowComplex::Imag;
    else
        throw std::runtime_error("Trying to set complex display to non-existent option");
}

void ImageTab::on_cmbComplex_currentIndexChanged(const QString &selection) {
    // the widget handles whether it has a plot or not
    ui->widget->setComplexDisplay(getComplexDisplayOption());
}
