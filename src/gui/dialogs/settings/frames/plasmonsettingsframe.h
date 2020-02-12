#ifndef PLASMONSETTINGSFRAME_H
#define PLASMONSETTINGSFRAME_H

#include <QWidget>
#include <simulationmanager.h>
#include <structure/structureparameters.h>


namespace Ui {
class PlasmonSettingsFrame;
}

class PlasmonSettingsFrame : public QWidget
{
    Q_OBJECT

private slots:
    void dlgOk_clicked();

    void dlgApply_clicked();

    void dlgCancel_clicked();

public:
    explicit PlasmonSettingsFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager);
    ~PlasmonSettingsFrame();

private:
    Ui::PlasmonSettingsFrame *ui;
};

#endif // PLASMONSETTINGSFRAME_H
