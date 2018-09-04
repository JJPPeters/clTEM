#ifndef AREALAYOUTFRAME_H
#define AREALAYOUTFRAME_H

#include <QWidget>
#include <simulationmanager.h>
#include "ctemareaframe.h"
#include "stemareaframe.h"
#include "cbedareaframe.h"
#include "controls/oglviewwidget/oglviewwidget.h"

namespace Ui {
class AreaLayoutFrame;
}

class AreaLayoutFrame : public QWidget
{
    Q_OBJECT

signals:
    void areaChanged();
    void resolutionChanged(QString);
    void modeChanged(int);
    void updateMainCbed();
    void updateMainStem();

public:
    explicit AreaLayoutFrame(QWidget *parent, std::shared_ptr<SimulationManager> simMan);
    ~AreaLayoutFrame();

private slots:
    void areasChanged();

    void checkEditZero(QString txt);

    bool apply_pressed();

    void on_cmbResolution_currentIndexChanged(const QString &arg1);

    void dlgOk_clicked();

    void dlgApply_clicked();

    void dlgCancel_clicked();

    void slicesChanged();

    void showEvent(QShowEvent* event) override;

private:
    Ui::AreaLayoutFrame *ui;

    std::shared_ptr<SimulationManager> SimManager;

    CtemAreaFrame *CtemFrame;
    StemAreaFrame *StemFrame;
    CbedAreaFrame *CbedFrame;

    OGLViewWidget *pltStructure;

    bool getErrorStringCtem();
    bool getErrorStringStem();
    bool getErrorStringCbed();

    void setStructLimits();

    void plotStructure();
};

#endif // AREALAYOUTFRAME_H
