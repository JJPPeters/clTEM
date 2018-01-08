#ifndef IMAGETAB_H
#define IMAGETAB_H

#include <QWidget>

#include "imageplot/imageplotwidget.h"

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

public:
    explicit ImageTab(QWidget *parent, std::string name, TabType t);

    ~ImageTab();

    std::string getTabName() {return TabName;}

    TabType getType() {return MyType;}

    ImagePlotWidget* getPlot();

private:
    Ui::ImageTab *ui;

    std::string TabName;

    TabType MyType;
};

#endif // IMAGETAB_H
