#ifndef IMAGETAB_H
#define IMAGETAB_H

#include <QWidget>

#include <ui_imagetab.h>

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
    void setPlotWithData(Image<T> img, nlohmann::json stngs, IntensityScale scale = IntensityScale::Linear, bool doReplot = true)
    {
        settings = stngs;
        ui->widget->SetImageTemplate(img, scale, doReplot);
    }

private:
    nlohmann::json settings;

    Ui::ImageTab *ui;

    std::string TabName;

    TabType MyType;

public slots:
    void forwardSaveData() { emit saveDataActivated(); }
    void forwardSaveImage() { emit saveImageActivated(); }
};

#endif // IMAGETAB_H
