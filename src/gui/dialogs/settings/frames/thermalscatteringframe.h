#ifndef THERMALSCATTERINGFRAME_H
#define THERMALSCATTERINGFRAME_H

#include <QWidget>
#include <utilities/commonstructs.h>

namespace Ui {
class ThermalScatteringFrame;
}

class ThermalScatteringFrame : public QWidget
{
    Q_OBJECT

public:
    explicit ThermalScatteringFrame(QWidget *parent);

    ~ThermalScatteringFrame();

private slots:
    void dlgOk_clicked();

    bool dlgApply_clicked();

    void dlgCancel_clicked();

private:
    Ui::ThermalScatteringFrame *ui;

};

#endif // THERMALSCATTERINGFRAME_H
