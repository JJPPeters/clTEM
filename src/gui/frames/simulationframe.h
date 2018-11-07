#ifndef SIMULATIONFRAME_H
#define SIMULATIONFRAME_H

#include <QWidget>
#include <mainwindow.h>

class MainWindow;

namespace Ui {
class SimulationFrame;
}

class SimulationFrame : public QWidget
{
    Q_OBJECT

signals:
    void resolutionSet(int);

public:
    explicit SimulationFrame(QWidget *parent = 0);

    ~SimulationFrame();

    void assignMainWindow(MainWindow* m);

    void updateStructureInfo(std::tuple<float, float, float, int> ranges);

    void updateResolutionInfo(float pixScale, float invScale, float invMax);

    void setResolutionIndex(int ind = 0);

    void setResolution(int res) {setResolutionText( QString::number(res) );}

public slots:
    void setResolutionText(QString text);

private slots:
    void on_cmbResolution_currentIndexChanged(const QString &arg1);

    void on_chkFull3D_toggled(bool checked);

    void on_chkFiniteDiff_toggled(bool checked);

    void on_btnSimArea_clicked();

//    void updateLimits();

private:
    Ui::SimulationFrame *ui;

    MainWindow* Main;
};

#endif // SIMULATIONFRAME_H
