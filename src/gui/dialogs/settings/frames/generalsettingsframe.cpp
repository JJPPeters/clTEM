#include <dialogs/settings/settingsdialog.h>
#ifdef _WIN32
#include <theme/thememanager.h>
#endif
#include "generalsettingsframe.h"
#include "ui_generalsettingsframe.h"
#include "utilities/logging.h"


GeneralSettingsFrame::GeneralSettingsFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager) :
    QWidget(parent), manager(simManager),
    ui(new Ui::GeneralSettingsFrame)
{
    ui->setupUi(this);

#ifdef _WIN32
    if (ThemeManager::CurrentTheme == ThemeManager::Theme::Dark)
        ui->cmbTheme->setCurrentText("Dark");
    else if (ThemeManager::CurrentTheme == ThemeManager::Theme::DarkGrey)
        ui->cmbTheme->setCurrentText("Dark-grey");
    else
        ui->cmbTheme->setCurrentText("Native");
#else
    ui->cmbTheme->setVisible(false);
    ui->lblTheme->setVisible(false);
    ui->cmbTheme->setEnabled(false);
    ui->lblTheme->setEnabled(false);
#endif

    QSettings settings;
    int msaa = settings.value("MSAA", 1).toInt();

    QString ms = QString::number(msaa);
    if (ms != "2" && ms != "4" && ms != "8")
        ms = "None";

    int ind = ui->cmbMultisampling->findText( ms );
    ui->cmbMultisampling->setCurrentIndex(ind);

    ui->chkLiveStem->setChecked(manager->liveStemEnabled());

    ui->chkLogging->setChecked(el::Loggers::getLogger("default")->configurations()->get(el::Level::Debug, el::ConfigurationType::ToFile)->value() == "true");

    auto parent_dlg = dynamic_cast<ThemeDialog*>(parentWidget());
    connect(parent_dlg, &ThemeDialog::okSignal, this, &GeneralSettingsFrame::dlgOk_clicked);
    connect(parent_dlg, &ThemeDialog::cancelSignal, this, &GeneralSettingsFrame::dlgCancel_clicked);
    connect(parent_dlg, &ThemeDialog::applySignal, this, &GeneralSettingsFrame::dlgApply_clicked);

}

GeneralSettingsFrame::~GeneralSettingsFrame()
{
    delete ui;
}

void GeneralSettingsFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    parentWidget()->close();
}

void GeneralSettingsFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    dlgApply_clicked();
    parentWidget()->close();
}

void GeneralSettingsFrame::dlgApply_clicked()
{
#ifdef _WIN32
    // Theme
    if (ui->cmbTheme->currentText() == "Dark")
        ThemeManager::setTheme(ThemeManager::Theme::Dark);
    else if (ui->cmbTheme->currentText() == "Dark-grey")
        ThemeManager::setTheme(ThemeManager::Theme::DarkGrey);
    else
        ThemeManager::setTheme(ThemeManager::Theme::Native);
#endif

    // logging
    std::string state_str = "false";
    bool state = ui->chkLogging->isChecked();
    if (state)
        state_str = "true";

    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::ToFile, state_str);
    el::Loggers::reconfigureAllLoggers(el::Level::Error, el::ConfigurationType::ToFile, "true");// ensure this is always logged

    int msaa = 1;
    if (ui->cmbMultisampling->currentText() != "None")
        msaa = std::stoi(ui->cmbMultisampling->currentText().toStdString());

    bool do_live = ui->chkLiveStem->isChecked();
    manager->setLiveStemEnabled(do_live);

    QSettings settings;
    settings.setValue("live stem", do_live);
    settings.setValue("logging", state);
    settings.setValue("MSAA", msaa);
}
