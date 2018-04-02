#include "temframe.h"
#include "ui_temframe.h"

#include <iostream>
#include <utils/stringutils.h>

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

void TemFrame::on_btnSim_clicked()
{
    emit startSim();
}

void TemFrame::setActive(bool active)
{
    ui->btnSim->setEnabled(active);
}

void TemFrame::on_btnCancel_clicked()
{
    emit stopSim();
}

void TemFrame::on_chkCrop_toggled(bool state) {
    emit setCtemCrop(state);
}

void TemFrame::setCropCheck(bool state) { ui->chkCrop->setChecked(state); }

void TemFrame::setSimImageCheck(bool state) { ui->chkSimImage->setChecked(state); }

void TemFrame::populateCcdCombo(std::vector<std::string> names){
    for (int i = 0; i < names.size(); ++i){
        ui->cmbCcd->addItem(QString::fromStdString(names[i]));
    }
}

void TemFrame::setCcdIndex(int index) {
    ui->cmbCcd->setCurrentIndex(index);
}

void TemFrame::setBinningIndex(int index) {
    ui->cmbBinning->setCurrentIndex(index);
}

void TemFrame::setDose(float dose)
{
    ui->edtDose->setText(Utils::numToQString((int) dose, edt_precision));
}

int TemFrame::getBinning() {
    return ui->cmbBinning->currentText().toInt();
}

std::string TemFrame::getCcd() {
    return ui->cmbCcd->currentText().toStdString();
}

bool TemFrame::getSimImage() {
    return ui->chkSimImage->isChecked();
}

float TemFrame::getDose() {
    return ui->edtDose->text().toFloat();
}
