#include "temframe.h"
#include "ui_temframe.h"

#include <iostream>

TemFrame::TemFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TemFrame)
{
    ui->setupUi(this);

    QRegExpValidator* pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));

    ui->edtDose->setValidator(pIntValidator);
}

TemFrame::~TemFrame()
{
    delete ui;
}

void TemFrame::on_edtDose_textChanged(const QString &arg1)
{
    if(arg1.toInt() < 1)
        ui->edtDose->setStyleSheet("color: #FF8C00");
    else
        ui->edtDose->setStyleSheet("");
}

void TemFrame::on_btnExitWave_clicked()
{
    emit startSim();
}

void TemFrame::setActive(bool active)
{
    ui->btnExitWave->setEnabled(active);
    ui->btnSim->setEnabled(active);
}

void TemFrame::on_btnCancel_clicked()
{
    emit stopSim();
}

void TemFrame::on_chkCrop_toggled(bool state) {
    emit setCtemCrop(state);
}

void TemFrame::setCrop(bool state) { ui->chkCrop->setChecked(state); }
