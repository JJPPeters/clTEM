#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <controls/statuslayout.h>
#include <frames/simulationframe.h>
#include <frames/stemframe.h>
#include <frames/cbedframe.h>

#include "simulationmanager.h"
#include "simulationthread.h"


class CbedFrame;
class SimulationFrame;
class StemFrame;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void sliceProgressUpdated(float);
    void totalProgressUpdated(float);

    void imagesReturned(std::map<std::string, Image<float>>, SimulationManager);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    std::shared_ptr<SimulationManager> Manager;

    std::shared_ptr<MicroscopeParameters> getMicroscopeParams() {return Manager->getMicroscopeParams();}
    std::shared_ptr<CrystalStructure> getStructure() {return Manager->getStructure();}
    std::shared_ptr<SimulationArea> getSimulationArea() {return Manager->getSimulationArea();}
    std::vector<StemDetector>& getDetectors() {return Manager->getDetectors();}
    std::shared_ptr<StemArea> getStemArea() {return Manager->getStemArea();}

    void setDetectors();

    void updateRanges();

    void updateSlicesProgress(float prog);
    void updateTotalProgress(float prog);

    void updateImages(std::map<std::string, Image<float>> ims, SimulationManager sm);

    void updateManagerFromGui();

    // these are to make conencting some signals/slots in dialogs much easier
    SimulationFrame* getSimulationFrame();
    StemFrame* getStemFrame();
    CbedFrame* getCbedFrame();

public slots:
    void updateScales();

    void updateVoltageMrad(float voltage);

    void set_active_mode(int mode);

private slots:
    void on_actionOpen_triggered();

    void on_actionOpenCL_triggered();

    void on_actionGeneral_triggered();

    void on_actionImport_parameters_triggered();

    void on_actionExport_parameters_triggered();

    void on_actionSet_area_triggered();

    void on_actionAberrations_triggered();

    void on_actionThermal_scattering_triggered();

    void on_actionSimulate_EW_triggered();

    void cancel_simulation();

    void set_ctem_crop(bool state);

    void resolution_changed(int resolution);

    void on_twMode_currentChanged(int index);

    void sliceProgressChanged(float prog);

    void totalProgressChanged(float prog);

    void imagesChanged(std::map<std::string, Image<float>> ims, SimulationManager sm);

    void setUiActive(bool active);

    void simulationComplete();

    void saveTiff();

    void saveBmp();

private:
    Ui::MainWindow *ui;

    StatusLayout* StatusBar;

    QMutex Progress_Mutex, Image_Mutex;

    std::shared_ptr<SimulationThread> SimThread;

    std::tuple<std::vector<clDevice>, std::vector<float>> Devices;

    void loadSavedOpenClSettings();

    bool checkSimulationPrerequisites();

    void loadExternalSources();

    void updateGuiFromManager();
};

#endif // MAINWINDOW_H
