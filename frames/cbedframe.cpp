#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
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

    float v = arg1.toFloat();

    Main->Manager->getCBedPosition()->setYPos(v);
}

void CbedFrame::on_edtPosX_textChanged(const QString &arg1)
{
    if (Main == 0)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    float v = arg1.toFloat();

    Main->Manager->getCBedPosition()->setXPos(v);
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

void CbedFrame::update_text_boxes()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    int edt_precision = 5;

    ui->edtPosX->setText( Utils::numToQString(Main->Manager->getCBedPosition()->getXPos(), edt_precision) );
    ui->edtPosY->setText( Utils::numToQString(Main->Manager->getCBedPosition()->getYPos(), edt_precision) );
}
