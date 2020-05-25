#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <controls/statuslayout.h>
#include <frames/simulationframe.h>
#include <frames/stemframe.h>
#include <frames/cbedframe.h>
#include <controls/flattitlebar.h>
#include <controls/borderlesswindow.h>
#include <controls/editunitsbox.h>

#include "simulationmanager.h"
#include "simulationthread.h"

#include "utilities/logging.h"
#include "utilities/simutils.h"

class CbedFrame;
class SimulationFrame;
class StemFrame;

namespace Ui {
class MainWindow;
}

class MainWindow : public BorderlessWindow
{
    Q_OBJECT

signals:
    void sliceProgressUpdated(double);
    void totalProgressUpdated(double);

    void imagesReturned(SimulationManager);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    std::shared_ptr<SimulationManager> Manager;

    void setDetectors();

    void updateRanges();

    void updateSlicesProgress(double prog);
    void updateTotalProgress(double prog);

    void updateImages(SimulationManager sm);

    void updateManagerFromGui();

    // these are to make conencting some signals/slots in dialogs much easier
    SimulationFrame* getSimulationFrame();
    StemFrame* getStemFrame();
    CbedFrame* getCbedFrame();

    void updateAberrationBoxes();

    void updateAberrationManager();

public slots:
    void updateScales();

    void updateVoltageMrad(double voltage);

    void set_active_mode(int mode);

private slots:
    void on_actionOpen_triggered();

    void on_actionOpenCL_triggered();

    void on_actionTheme_triggered();

    void on_actionGeneral_triggered();

    void on_actionImport_parameters_triggered();

    void on_actionExport_parameters_triggered();

    void on_actionImport_default_triggered(bool preserve_ui = true);

    void on_actionExport_default_triggered();

    void on_actionShow_default_triggered();

    void on_actionSet_area_triggered();

    void on_actionAberrations_triggered();

    void on_actionThermal_scattering_triggered();

    void on_actionPlasmons_triggered();

    void on_actionSimulate_EW_triggered();

    void cancel_simulation();

    void set_ctem_crop(bool state);

    void resolution_changed(int resolution);

    void on_twMode_currentChanged(int index);

    void sliceProgressChanged(double prog);

    void totalProgressChanged(double prog);

    void imagesChanged(SimulationManager sm);

    void setUiActive(bool active);

    void simulationComplete();

    void simulationFailed();

    void saveTiff(bool full_stack);

    void saveBmp(bool full_stack);

    void ctemImageToggled(bool state) {
        Manager->setCtemImageEnabled(state);
        updateModeTextBoxes();
    }

    void updateModeTextBoxes();

    void checkEditZero(QString dud)
    {
        (void)dud; // make it explicit that this is not used

        auto * edt = dynamic_cast<EditUnitsBox*>(sender());

        if(edt == nullptr)
            return;

        double val = edt->text().toDouble();

        if (val <= 0)
            edt->setStyleSheet("color: #FF8C00"); // I just chose orange, mgiht want to be a better colour
        else
            edt->setStyleSheet("");
    }

private:
    FlatTitleBar *m_title;

    StatusLayout* StatusBar;

    QMutex Progress_Mutex, Image_Mutex;

    std::shared_ptr<SimulationThread> SimThread;

    std::vector<clDevice> Devices;

    void loadSavedOpenClSettings();

    void loadExternalSources();

    void updateGuiFromManager();

protected:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
