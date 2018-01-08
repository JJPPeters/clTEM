#ifndef SIMAREAFRAME_H
#define SIMAREAFRAME_H

#include <QWidget>
#include <QtWidgets/QLineEdit>
#include <utilities/commonstructs.h>
#include <structure/crystalstructure.h>

namespace Ui {
class SimAreaFrame;
}

class SimAreaFrame : public QWidget
{
    Q_OBJECT

signals:
    void areaChanged();

public:
    explicit SimAreaFrame(QWidget *parent, std::shared_ptr<SimulationArea> sa, std::shared_ptr<CrystalStructure> struc);

    ~SimAreaFrame();

private slots:
    void on_btnReset_clicked();

    void checkEditZeroX(QString dud);
    void checkEditZeroY(QString dud);

    void dlgOk_clicked();

    bool dlgApply_clicked();

    void dlgCancel_clicked();

private:
    Ui::SimAreaFrame *ui;

//    float xRange, yRange;

    std::shared_ptr<SimulationArea> simArea;

    std::shared_ptr<CrystalStructure> structure;

    void checkEditZero(QLineEdit* edtStart, QLineEdit* edtFinish);

};

#endif // SIMAREAFRAME_H
