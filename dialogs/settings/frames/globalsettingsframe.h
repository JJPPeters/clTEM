#ifndef GLOBALSETTINGSFRAME_H
#define GLOBALSETTINGSFRAME_H

#include <QWidget>
#include <simulation/simulationmanager.h>

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

public:
    explicit GlobalSettingsFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager);
    ~GlobalSettingsFrame();

private:
    Ui::GlobalSettingsFrame *ui;

    std::shared_ptr<SimulationManager> Manager;
};

#endif // GLOBALSETTINGSFRAME_H
