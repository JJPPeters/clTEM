#ifndef IMAGETAB_H
#define IMAGETAB_H

#include <QWidget>

#include <ui_imagetab.h>
#include <simulationmanager.h>

#include "imageplot/imageplotwidget.h"
#include "utilities/json.hpp"

enum class TabType{
    CTEM,
    STEM,
    DIFF};

namespace Ui {
class ImageTab;
}

class ImageTab : public QWidget
{
    Q_OBJECT

signals:
    void saveDataActivated(bool);
    void saveImageActivated(bool);

public:
    explicit ImageTab(QWidget *parent, std::string name, TabType t, bool is_complex = false);

    ~ImageTab();

    std::string getTabName() {return TabName;}

    TabType getType() {return MyType;}

    ImagePlotWidget* getPlot();

    nlohmann::json getSettings();

    template <typename T>
    void setPlotWithData(Image<T> img,
                         QString units, double sc_x, double sc_y, double lo_x, double lo_y,
                         nlohmann::json stngs,
                         IntensityScale scale = IntensityScale::Linear,
                         ZeroPosition zp = ZeroPosition::BottomLeft,
                         bool doReplot = true)
    {
        settings = stngs;
        image_units = units;
        if (img.getDepth() > 1) {
            ui->hScrlSlice->setMinimum(0);
            ui->hScrlSlice->setMaximum(img.getDepth() - 1);

            ui->hScrlSlice->setVisible(true);
            disconnect(ui->hScrlSlice, &QScrollBar::valueChanged, this, &ImageTab::sliceSliderChanged);
            ui->hScrlSlice->setValue(img.getDepth() - 1);
            connect(ui->hScrlSlice, &QScrollBar::valueChanged, this, &ImageTab::sliceSliderChanged);
        } else
            ui->hScrlSlice->setVisible(false);

        ui->widget->SetImage(img, lo_x, lo_y, sc_x, sc_y, scale, zp, doReplot);
    }

    template <typename T>
    void setPlotWithComplexData(Image<std::complex<T>> img,
                                QString units, double sc_x, double sc_y, double lo_x, double lo_y,
                                nlohmann::json stngs,
                                IntensityScale scale = IntensityScale::Linear,
                                ZeroPosition zp = ZeroPosition::BottomLeft,
                                bool doReplot = true)
    {
        settings = stngs;
        image_units = units;
        if (img.getDepth() > 1) {
            ui->hScrlSlice->setMinimum(0);
            ui->hScrlSlice->setMaximum(img.getDepth() - 1);

            ui->hScrlSlice->setVisible(true);
            disconnect(ui->hScrlSlice, &QScrollBar::valueChanged, this, &ImageTab::sliceSliderChanged);
            ui->hScrlSlice->setValue(img.getDepth() - 1);
            connect(ui->hScrlSlice, &QScrollBar::valueChanged, this, &ImageTab::sliceSliderChanged);
        } else
            ui->hScrlSlice->setVisible(false);

        ui->widget->SetComplexImage(img, lo_x, lo_y, sc_x, sc_y, scale, getComplexDisplayOption(), zp, doReplot);
    }

private:
    nlohmann::json settings;

    Ui::ImageTab *ui;

    std::string TabName;

    TabType MyType;

    QString image_units;

    ShowComplex getComplexDisplayOption();

    void sliceSliderChanged(int value);

public slots:
    void forwardSaveData(bool full_stack) { emit saveDataActivated(full_stack); }
    void forwardSaveImage(bool full_stack) { emit saveImageActivated(full_stack); }

    void on_cmbComplex_currentIndexChanged(const QString &selection);

private slots:
    void updatePositionLabels(double x, double y);
};

#endif // IMAGETAB_H
