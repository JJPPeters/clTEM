#ifndef STEMAREAFRAME_H
#define STEMAREAFRAME_H

#include <QWidget>
#include <utilities/commonstructs.h>
#include <QtWidgets/QLineEdit>

namespace Ui {
class StemAreaFrame;
}

class StemAreaFrame : public QWidget
{
    Q_OBJECT

signals:
    void areaChanged();

public:
//    explicit StemAreaFrame(QWidget *parent, std::shared_ptr<StemArea> stem, std::shared_ptr<SimulationArea> sim);
    explicit StemAreaFrame(QWidget *parent, StemArea sa, std::shared_ptr<CrystalStructure> struc);

    ~StemAreaFrame();

    StemArea stemArea();

    void updateCurrentArea(StemArea new_area) {Area = new_area;}

private slots:
    void valuesChanged(QString dud);

    void xStartRangeChanged(QString dud);
    void xFinishChanged(QString dud);

    void yStartRangeChanged(QString dud);
    void yFinishChanged(QString dud);

    void on_btnReset_clicked();

    void on_btnDefault_clicked();

    void editing_finished();

private:
    Ui::StemAreaFrame *ui;

    StemArea Area;

    std::shared_ptr<CrystalStructure> Structure;

    bool checkValidXValues();
    bool checkValidYValues();
    void setInvalidXWarning(bool valid);
    void setInvalidYWarning(bool valid);
};

#endif // STEMAREAFRAME_H
