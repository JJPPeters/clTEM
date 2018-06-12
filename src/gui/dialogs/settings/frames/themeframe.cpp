#include <dialogs/settings/settingsdialog.h>
#include <theme/thememanager.h>
#include "themeframe.h"
#include "ui_themeframe.h"

ThemeFrame::ThemeFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThemeFrame)
{
    ui->setupUi(this);

    if (ThemeManager::CurrentTheme == ThemeManager::Theme::Dark)
        ui->cmbTheme->setCurrentText("Dark");
    else
        ui->cmbTheme->setCurrentText("Native");

    auto parent_dlg = dynamic_cast<ThemeDialog*>(parentWidget());
    connect(parent_dlg, &ThemeDialog::okSignal, this, &ThemeFrame::dlgOk_clicked);
    connect(parent_dlg, &ThemeDialog::cancelSignal, this, &ThemeFrame::dlgCancel_clicked);
    connect(parent_dlg, &ThemeDialog::applySignal, this, &ThemeFrame::dlgApply_clicked);
}

ThemeFrame::~ThemeFrame()
{
    delete ui;
}

void ThemeFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    parentWidget()->close();
}

void ThemeFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    dlgApply_clicked();
    parentWidget()->close();
}

void ThemeFrame::dlgApply_clicked()
{
    if (ui->cmbTheme->currentText() == "Native")
        ThemeManager::setTheme(ThemeManager::Theme::Native);
    else if (ui->cmbTheme->currentText() == "Dark")
        ThemeManager::setTheme(ThemeManager::Theme::Dark);
}
