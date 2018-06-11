#include <dialogs/settings/settingsdialog.h>
#include "themeframe.h"
#include "ui_themeframe.h"

ThemeFrame::ThemeFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThemeFrame)
{
    ui->setupUi(this);

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

}

void ThemeFrame::on_cmbTheme_currentIndexChanged(int index) {
    std::string text = ui->cmbTheme->currentText().toStdString();
}
