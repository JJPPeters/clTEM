#ifndef THERMALSCATTERINGFRAME_H
#define THERMALSCATTERINGFRAME_H

#include <QWidget>
#include <utilities/commonstructs.h>
#include <simulationmanager.h>

namespace Ui {
class ThermalScatteringFrame;
}

class ThermalScatteringFrame : public QWidget
{
    Q_OBJECT

signals:
    void phononsApplied();

public:
    explicit ThermalScatteringFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager);

    ~ThermalScatteringFrame();

private slots:
    void dlgOk_clicked();

    bool dlgApply_clicked();

    void dlgCancel_clicked();

    void on_btnAdd_clicked();

    void on_btnDelete_clicked();

private:
    Ui::ThermalScatteringFrame *ui;

//    std::shared_ptr<SimulationManager> Manager;

    std::shared_ptr<PhononScattering> Phonons;

    void addItemToList(std::string el, double vib);

};

#endif // THERMALSCATTERINGFRAME_H
