#include <dialogs/settings/settingsdialog.h>
#ifdef _WIN32
#include <theme/thememanager.h>
#endif
#include "plasmonsettingsframe.h"
#include "ui_plasmonsettingsframe.h"
#include "utilities/logging.h"


PlasmonSettingsFrame::PlasmonSettingsFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager) :
    QWidget(parent),
    ui(new Ui::PlasmonSettingsFrame)
{
    ui->setupUi(this);
}

PlasmonSettingsFrame::~PlasmonSettingsFrame()
{
    delete ui;
}

void PlasmonSettingsFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    parentWidget()->close();
}

void PlasmonSettingsFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    dlgApply_clicked();
    parentWidget()->close();
}

void PlasmonSettingsFrame::dlgApply_clicked()
{

}
