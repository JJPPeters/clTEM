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
    void saveDataActivated();
    void saveImageActivated();

public:
    explicit ImageTab(QWidget *parent, std::string name, TabType t);

    ~ImageTab();

    std::string getTabName() {return TabName;}

    TabType getType() {return MyType;}

    ImagePlotWidget* getPlot();

    nlohmann::json getSettings();

    template <typename T>
    void setPlotWithData(Image<T> img, QString units, double sc_x, double sc_y, double lo_x, double lo_y, nlohmann::json stngs, IntensityScale scale = IntensityScale::Linear, ZeroPosition zp = ZeroPosition::BottomLeft, bool doReplot = true)
    {
        settings = stngs;
        image_units = units;
        ui->widget->SetImage(img, lo_x, lo_y, sc_x, sc_y, scale, zp, doReplot);
    }

private:
    nlohmann::json settings;

    Ui::ImageTab *ui;

    std::string TabName;

    TabType MyType;

    QString image_units;

public slots:
    void forwardSaveData() { emit saveDataActivated(); }
    void forwardSaveImage() { emit saveImageActivated(); }

private slots:
    void updatePositionLabels(double x, double y);
};

#endif // IMAGETAB_H
