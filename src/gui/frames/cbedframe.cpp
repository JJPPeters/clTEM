#include "cbedframe.h"
#include "ui_cbedframe.h"

#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>

CbedFrame::CbedFrame(QWidget *parent) :
        QWidget(parent), Main(0),
        ui(new Ui::CbedFrame)
{
    ui->setupUi(this);

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));
    QRegExpValidator* pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));

    ui->edtPosX->setValidator(pmValidator);
    ui->edtPosY->setValidator(pmValidator);
    ui->edtTds->setValidator(pIntValidator);

    ui->edtPosX->setUnits("Å");
    ui->edtPosY->setUnits("Å");

    connect(ui->edtTds, &QLineEdit::textChanged, this, &CbedFrame::edtTds_changed);
}

CbedFrame::~CbedFrame()
{
    delete ui;
}

void CbedFrame::edtTds_changed_proxy(const QString &arg1, bool update_partner) {
    // due to the complexities of this interacting with the STEM version, this will be set later (when the sim in run)
    int v = arg1.toInt();

    if(v < 1)
        ui->edtTds->setStyleSheet("color: #FF8C00");
    else
        ui->edtTds->setStyleSheet("");

    if (update_partner)
        Main->getStemFrame()->setTdsRuns(arg1.toUInt());
}

void CbedFrame::edtTds_changed(const QString &arg1)
{
    edtTds_changed_proxy(arg1, true);
}

void CbedFrame::setTdsRuns(unsigned int runs) {
    disconnect(ui->edtTds, &QLineEdit::textChanged, this, &CbedFrame::edtTds_changed);
    auto new_num = QString::number(runs);
    ui->edtTds->setText(new_num);
    edtTds_changed_proxy(new_num, false);
    connect(ui->edtTds, &QLineEdit::textChanged, this, &CbedFrame::edtTds_changed);
}

void CbedFrame::on_edtPosY_textChanged(const QString &arg1)
{
    (void)arg1; // don't want this

    if (Main == nullptr)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    double v = ui->edtPosY->text().toDouble();

    Main->Manager->getCBedPosition()->setYPos(v);
}

void CbedFrame::on_edtPosX_textChanged(const QString &arg1)
{
    (void)arg1; // don't want this

    if (Main == nullptr)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    double v = ui->edtPosX->text().toDouble();

    Main->Manager->getCBedPosition()->setXPos(v);
}

void CbedFrame::on_btnSim_clicked()
{
    if (Main == nullptr)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    emit startSim();
}

void CbedFrame::on_chkTds_stateChanged(int state)
{
    // this just updates the other frame to have the same state
    Main->getStemFrame()->setTdsEnabled(state != 0);
}


void CbedFrame::setTdsEnabled(bool enabled)
{
    ui->chkTds->setChecked(enabled);
}

bool CbedFrame::isTdsEnabled()
{
    return ui->chkTds->checkState() == Qt::Checked;
}

unsigned int CbedFrame::getTdsRuns()
{
    return ui->edtTds->text().toUInt();
}

void CbedFrame::setActive(bool active)
{
    ui->btnSim->setEnabled(active);
}

void CbedFrame::on_btnCancel_clicked()
{
    emit stopSim();
}

void CbedFrame::updateTextBoxes()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    ui->edtPosX->setText( Utils_Qt::numToQString(Main->Manager->getCBedPosition()->getXPos()) );
    ui->edtPosY->setText( Utils_Qt::numToQString(Main->Manager->getCBedPosition()->getYPos()) );
    ui->edtTds->setText( Utils_Qt::numToQString(Main->Manager->getInelasticScattering()->getStoredInelasticInterations()));
}

void CbedFrame::updateTds() {
    ui->chkTds->setChecked( Main->Manager->getInelasticScattering()->getPhonons()->getFrozenPhononEnabled());
}