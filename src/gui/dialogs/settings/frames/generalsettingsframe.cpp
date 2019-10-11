#include <dialogs/settings/settingsdialog.h>
#include <theme/thememanager.h>
#include "generalsettingsframe.h"
#include "ui_generalsettingsframe.h"
#include "utilities/logging.h"


GeneralSettingsFrame::GeneralSettingsFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GeneralSettingsFrame)
{
    ui->setupUi(this);

    if (ThemeManager::CurrentTheme == ThemeManager::Theme::Dark)
        ui->cmbTheme->setCurrentText("Dark");
    else if (ThemeManager::CurrentTheme == ThemeManager::Theme::Light)
        ui->cmbTheme->setCurrentText("Light");
    else
        ui->cmbTheme->setCurrentText("Native");

    QSettings settings;
    int msaa = settings.value("MSAA", 1).toInt();

    QString ms = QString::number(msaa);
    if (ms != "2" && ms != "4" && ms != "8")
        ms = "None";

    int ind = ui->cmbMultisampling->findText( ms );
    ui->cmbMultisampling->setCurrentIndex(ind);

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
    // Theme
    if (ui->cmbTheme->currentText() == "Dark")
        ThemeManager::setTheme(ThemeManager::Theme::Dark);
    else if (ui->cmbTheme->currentText() == "Light")
        ThemeManager::setTheme(ThemeManager::Theme::Light);
    else
        ThemeManager::setTheme(ThemeManager::Theme::Native);

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

    QSettings settings;
    settings.setValue("logging", state);
    settings.setValue("MSAA", msaa);
}
