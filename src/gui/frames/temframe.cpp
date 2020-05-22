#include "temframe.h"
#include "ui_temframe.h"

#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include <QScreen>

TemFrame::TemFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TemFrame)
{
    ui->setupUi(this);

    QScreen* primary_screen = QGuiApplication::primaryScreen();
    double pixel_ratio = primary_screen->devicePixelRatio();

    int col1 = 75  / pixel_ratio;
//    int col2 = 100 / pixel_ratio;
//    int col3 = 100 / pixel_ratio;

    auto test = dynamic_cast<QGridLayout*>(this->layout());

    test->setColumnMinimumWidth(0, col1);
//    test->setColumnMinimumWidth(1, col2);
//    test->setColumnMinimumWidth(2, col3);

    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtDose->setValidator(pValidator);

    ui->edtDose->setUnits("e⁻A⁻²");


}

TemFrame::~TemFrame()
{
    delete ui;
}

void TemFrame::on_edtDose_textChanged(const QString &arg1)
{
    (void)arg1; // don't want this.
    auto v = ui->edtDose->text();

    if(v.toDouble() > 0.0)
        ui->edtDose->setStyleSheet("");
    else
        ui->edtDose->setStyleSheet("color: #FF8C00");
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

void TemFrame::setDose(double dose)
{
    ui->edtDose->setText(Utils_Qt::numToQString((int) dose));
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

double TemFrame::getDose() {
    return ui->edtDose->text().toDouble();
}

void TemFrame::update_ccd_boxes(std::shared_ptr<SimulationManager> sm) {
    double dse = sm->ccdDose();
    auto nm = sm->ccdName();
    int bn = sm->ccdBinning();

    ui->edtDose->setText( Utils_Qt::numToQString(dse) );

    // TODO: could use findtext?
    // set the name if it exists...

    int ind = ui->cmbCcd->findText( QString::fromStdString(nm) );
    ind += (ind == -1);
    ui->cmbCcd->setCurrentIndex(ind);

    ind = ui->cmbBinning->findText( QString::number(bn) );
    ind += (ind == -1);
    ui->cmbBinning->setCurrentIndex(ind);
}
