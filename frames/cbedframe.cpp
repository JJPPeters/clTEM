#include "cbedframe.h"
#include "ui_cbedframe.h"

CbedFrame::CbedFrame(QWidget *parent) :
    QWidget(parent), Main(0),
    ui(new Ui::CbedFrame)
{
    ui->setupUi(this);

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp("[+-]?(\\d*(?:\\.\\d*)?(?:[eE]([+\\-]?\\d+)?)>)*"));
    QRegExpValidator* pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));

    ui->edtPosX->setValidator(pmValidator);
    ui->edtPosY->setValidator(pmValidator);
    ui->edtTds->setValidator(pIntValidator);
}

CbedFrame::~CbedFrame()
{
    delete ui;
}

void CbedFrame::on_edtTds_textChanged(const QString &arg1)
{
    // due to the complexities of this interacting with the STEM version, this will be set later (when the sim in run)
    int v = arg1.toInt();

    if(v < 1)
        ui->edtTds->setStyleSheet("color: #FF8C00");
    else
        ui->edtTds->setStyleSheet("");
}

void CbedFrame::on_edtPosY_textChanged(const QString &arg1)
{
    if (Main == 0)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    auto r = Main->getSimulationArea()->getLimitsY();

    float v = arg1.toFloat();

    if (v < r[0] || v > r[1])
        ui->edtPosY->setStyleSheet("color: #FF8C00");
    else
    {
        ui->edtPosY->setStyleSheet("");
        Main->Manager->getCBedPosition()->setYPos(v);
    }
}

void CbedFrame::on_edtPosX_textChanged(const QString &arg1)
{
    if (Main == 0)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    auto r = Main->getSimulationArea()->getLimitsX();

    float v = arg1.toFloat();

    if (v < r[0] || v > r[1])
        ui->edtPosX->setStyleSheet("color: #FF8C00");
    else
    {
        ui->edtPosX->setStyleSheet("");
        Main->Manager->getCBedPosition()->setXPos(v);
    }
}

void CbedFrame::on_btnSim_clicked()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    emit startSim();
}

void CbedFrame::on_chkTds_stateChanged(int state)
{
    // This function is implemented when the simulation has been executed (to avoid complication due to CBED and STEM
    // both having the same checkbox, but only one variable to store it in
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
