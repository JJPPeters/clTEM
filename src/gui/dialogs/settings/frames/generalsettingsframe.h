#ifndef GENERALSETTINGSFRAME_H
#define GENERALSETTINGSFRAME_H

#include <QWidget>
#include <simulationmanager.h>
#include <structure/structureparameters.h>


namespace Ui {
class GeneralSettingsFrame;
}

class GeneralSettingsFrame : public QWidget
{
    Q_OBJECT

private slots:
    void dlgOk_clicked();

    void dlgApply_clicked();

    void dlgCancel_clicked();

public:
    explicit GeneralSettingsFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager);
    ~GeneralSettingsFrame();

private:
    std::shared_ptr<SimulationManager> manager;

    Ui::GeneralSettingsFrame *ui;
};

#endif // GENERALSETTINGSFRAME_H
