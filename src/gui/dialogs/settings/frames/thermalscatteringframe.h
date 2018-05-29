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

    void on_btnAdd_clicked();

    void on_btnDelete_clicked();

    void on_chkForceDefault_toggled(bool checked);

    void on_chkOverride_toggled(bool checked);

private:
    Ui::ThermalScatteringFrame *ui;

    void addItemToList(std::string el, float vib);

};

#endif // THERMALSCATTERINGFRAME_H
