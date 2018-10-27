#ifndef SIMAREAFRAME_H
#define SIMAREAFRAME_H

#include <QWidget>
#include <QtWidgets/QLineEdit>
#include <utilities/commonstructs.h>
#include <structure/crystalstructure.h>

namespace Ui {
class CtemAreaFrame;
}

class CtemAreaFrame : public QWidget
{
    Q_OBJECT

signals:
    void areaChanged();
    void applyChanges();

public:
    explicit CtemAreaFrame(QWidget *parent, SimulationArea sa);//, std::shared_ptr<SimulationArea> sa, std::shared_ptr<CrystalStructure> struc);

    ~CtemAreaFrame();

    SimulationArea getSimArea();

    void updateCurrentArea(SimulationArea new_area) {simArea = new_area;}

private slots:
    void xFinishChanged(QString dud);
    void yFinishChanged(QString dud);

    void xRangeChanged(QString dud);
    void yRangeChanged(QString dud);

    void on_btnReset_clicked();

    void on_btnApply_clicked() {emit applyChanges();}

    void editing_finished();

private:
    Ui::CtemAreaFrame *ui;

    SimulationArea simArea;

    bool checkXValid();
    bool checkYValid();

    void setXInvalidWarning(bool valid);
    void setYInvalidWarning(bool valid);
};

#endif // SIMAREAFRAME_H
