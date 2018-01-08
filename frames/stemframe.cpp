#include <dialogs/settings/settingsdialog.h>
#include <utilities/stringutils.h>
#include "stemframe.h"
#include "ui_stemframe.h"

StemFrame::StemFrame(QWidget *parent) :
    QWidget(parent), Main(0),
    ui(new Ui::StemFrame)
{
    ui->setupUi(this);

    QRegExpValidator* pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));

    ui->edtTds->setValidator(pIntValidator);
}

StemFrame::~StemFrame()
{
    delete ui;
}

void StemFrame::on_btnDetectors_clicked()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting STEM frame to main window.");
    StemDetectorDialog* myDialog = new StemDetectorDialog(this, Main->getDetectors());

    connect(myDialog, SIGNAL(detectorsChanged()), this, SLOT(updateDetectors()));

    myDialog->exec();
}

void StemFrame::updateDetectors()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting STEM frame to main window.");
    Main->setDetectors();
}

void StemFrame::on_btnArea_clicked()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting STEM frame to main window.");

    StemAreaDialog* myDialog = new StemAreaDialog(this, Main->getStemArea(), Main->getSimulationArea());

    connect(myDialog, SIGNAL(stemAreaChanged()), this, SLOT(updateScaleLabels()));

    myDialog->exec();
}

void StemFrame::updateScaleLabels()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting STEM frame to main window.");

    float scaleX = Main->getStemArea()->getScaleX();
    float scaleY = Main->getStemArea()->getScaleY();

    ui->lblStemScaleX->setText( "x: " + QString::fromStdString( Utils::numToString(scaleX, 2) + " Å" ) );
    ui->lblStemScaleY->setText( "y: " + QString::fromStdString( Utils::numToString(scaleY, 2) + " Å" ) );
}

void StemFrame::on_edtTds_textChanged(const QString &arg1)
{
    if(arg1.toInt() < 1)
        ui->edtTds->setStyleSheet("color: #FF8C00");
    else
        ui->edtTds->setStyleSheet("");
}

void StemFrame::on_btnSim_clicked()
{
    emit startSim();
}

bool StemFrame::isTdsEnabled()
{
    return ui->chkTds->checkState() == Qt::Checked;
}

unsigned int StemFrame::getTdsRuns()
{
    return ui->edtTds->text().toUInt();
}

void StemFrame::setActive(bool active)
{
    ui->btnSim->setEnabled(active);
}

void StemFrame::on_btnCancel_clicked()
{
    emit stopSim();
}
