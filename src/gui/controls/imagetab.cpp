#include <utils/stringutils.h>
#include "imagetab.h"
#include "ui_imagetab.h"

ImageTab::ImageTab(QWidget *parent, std::string name, TabType t, bool is_complex) :
    QWidget(parent), ui(new Ui::ImageTab), TabName(name), MyType(t)
{
    ui->setupUi(this);
    ui->hScrlSlice->setVisible(false);

    if (!is_complex) {
        ui->cmbComplex->setEnabled(false);
        ui->cmbComplex->setStyleSheet("QComboBox {min-width: 0px; border-left: 0px; border-right: 0px; padding-left: 0px; padding-right: 0px;}");
        ui->cmbComplex->setFixedWidth(0); // do this, so the ui doesnt resize
    }

    connect(ui->hScrlSlice, &QScrollBar::valueChanged, this, &ImageTab::sliceSliderChanged);

    connect(ui->widget, &ImagePlotWidget::saveDataClicked, this, &ImageTab::forwardSaveData);
    connect(ui->widget, &ImagePlotWidget::saveImageClicked, this, &ImageTab::forwardSaveImage);

    connect(ui->widget, &ImagePlotWidget::mouseHoverEvent, this, &ImageTab::updatePositionLabels);

    ui->widget->setMinimumHeight(500);
    ui->widget->setMinimumWidth(500);
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

void ImageTab::sliceSliderChanged(int value) {
    ui->widget->setSlice(value);
}
