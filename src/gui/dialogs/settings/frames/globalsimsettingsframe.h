#ifndef GLOBALSIMSETTINGSFRAME_H
#define GLOBALSIMSETTINGSFRAME_H

#include <QWidget>
#include <simulationmanager.h>
#include <structure/structureparameters.h>


namespace Ui {
class GlobalSimSettingsFrame;
}

class GlobalSimSettingsFrame : public QWidget
{
    Q_OBJECT

private slots:
    void dlgOk_clicked();

    void dlgApply_clicked();

    void dlgCancel_clicked();

    void checkValidInputs();

    void checkLiveChanged(int dud) {}

public:
    explicit GlobalSimSettingsFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager);
    ~GlobalSimSettingsFrame();

private:
    Ui::GlobalSimSettingsFrame *ui;

    std::shared_ptr<SimulationManager> Manager;

    void populateParamsCombo();
};

#endif // GLOBALSIMSETTINGSFRAME_H
