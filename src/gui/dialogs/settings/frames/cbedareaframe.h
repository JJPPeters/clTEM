#ifndef CBEDAREAFRAME_H
#define CBEDAREAFRAME_H

#include <QWidget>
#include <utilities/commonstructs.h>
#include <structure/crystalstructure.h>

namespace Ui {
class CbedAreaFrame;
}

class CbedAreaFrame : public QWidget
{
    Q_OBJECT

signals:
    void areaChanged();

public:
    explicit CbedAreaFrame(QWidget *parent, CbedPosition pos, std::shared_ptr<CrystalStructure> struc);
    ~CbedAreaFrame();

    CbedPosition getCbedPos();

    void updateCurrentArea(CbedPosition new_area) {Position = new_area;}

private slots:
    void valuesChanged(QString dud);

    void on_btnReset_clicked();

    void on_btnDefault_clicked();

    void editing_finished();

private:
    Ui::CbedAreaFrame *ui;

    CbedPosition Position;

    std::shared_ptr<CrystalStructure> Structure;
};

#endif // CBEDAREAFRAME_H
