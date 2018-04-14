#ifndef GLOBALSETTINGSFRAME_H
#define GLOBALSETTINGSFRAME_H

#include <QWidget>
#include <simulationmanager.h>
#include <structure/structureparameters.h>


namespace Ui {
class GlobalSettingsFrame;
}

class GlobalSettingsFrame : public QWidget
{
    Q_OBJECT

private slots:
    void dlgOk_clicked();

    void dlgApply_clicked();

    void dlgCancel_clicked();

    void on_cmbParams_currentIndexChanged(int index);

public:
    explicit GlobalSettingsFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager);
    ~GlobalSettingsFrame();

private:
    Ui::GlobalSettingsFrame *ui;

    std::shared_ptr<SimulationManager> Manager;

    void populateParamsCombo();
};

#endif // GLOBALSETTINGSFRAME_H
