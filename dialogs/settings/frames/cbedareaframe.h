#ifndef CBEDAREAFRAME_H
#define CBEDAREAFRAME_H

#include <QWidget>
#include <simulation/utilities/commonstructs.h>

namespace Ui {
class CbedAreaFrame;
}

class CbedAreaFrame : public QWidget
{
    Q_OBJECT

signals:
    void areaChanged();
    void applyChanges();

public:
    explicit CbedAreaFrame(QWidget *parent, CbedPosition pos);
    ~CbedAreaFrame();

    CbedPosition getCbedPos();

    void updateCurrentArea(CbedPosition new_area) {Position = new_area;}

private slots:
    void valuesChanged(QString dud);

    void on_btnReset_clicked();

    void on_btnApply_clicked() {emit applyChanges();}

    void editing_finished();

private:
    Ui::CbedAreaFrame *ui;

    CbedPosition Position;

    int edt_precision = 5;
};

#endif // CBEDAREAFRAME_H
