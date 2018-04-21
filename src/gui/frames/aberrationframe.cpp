#include "aberrationframe.h"
#include "ui_aberrationframe.h"

#include <utilities/stringutils.h>
#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>

#include "dialogs/settings/settingsdialog.h"

AberrationFrame::AberrationFrame(QWidget *parent) :
    QWidget(parent), Main(nullptr),
    ui(new Ui::AberrationForm)
{
    ui->setupUi(this);

    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));
    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtDefocus->setValidator(pmValidator);
    ui->edtSphere->setValidator(pmValidator);
    ui->edtStigMag->setValidator(pmValidator);
    ui->edtStigAng->setValidator(pmValidator);

    ui->edtAperture->setValidator(pValidator);
    ui->edtDelta->setValidator(pValidator);
    ui->edtConverge->setValidator(pValidator);
    ui->edtVoltage->setValidator(pValidator);

    // these connect to a slot that makes the text colour change if the value is zero
    // I don't want to disable 0 in the regex as people might want it as a leading character (e.g. 0.1)
    connect(ui->edtAperture, SIGNAL(textChanged(QString)), this, SLOT(checkEditZero(QString)));
    connect(ui->edtDelta, SIGNAL(textChanged(QString)), this, SLOT(checkEditZero(QString)));
    connect(ui->edtConverge, SIGNAL(textChanged(QString)), this, SLOT(checkEditZero(QString)));
    connect(ui->edtVoltage, SIGNAL(textChanged(QString)), this, SLOT(checkEditZero(QString)));
}

AberrationFrame::~AberrationFrame()
{
    delete ui;
}

void AberrationFrame::checkEditZero(QString dud)
{
    (void)dud; // we don't use this

    auto * edt = dynamic_cast<QLineEdit*>(sender());

    if(edt == nullptr)
        return;

    float val = edt->text().toFloat();

    if (val <= 0)
        edt->setStyleSheet("color: #FF8C00"); // I just chose orange, mgiht want to be a better colour
    else
        edt->setStyleSheet("");
}

void AberrationFrame::on_btnMore_clicked()
{
    if (Main == nullptr)
        throw std::runtime_error("Error connecting aberration frame to main window.");

    updateAberrations(); // here we update the current aberrations from the text boxes here so the dialog can show the same

    AberrationsDialog* myDialog = new AberrationsDialog(this, Main->getMicroscopeParams());

    // how this is dosconnected when the dialog is destroyed...
    connect(myDialog, SIGNAL(aberrationsChanged()), this, SLOT(updateTextBoxes()));

    // we don't need to try and get anything from this dialog as it just updates the pointers we gave it!
    myDialog->exec();
}

void AberrationFrame::updateTextBoxes()
{
    if (Main == nullptr)
        throw std::runtime_error("Error connecting aberration frame to main window.");
    auto p = Main->Manager->getMicroscopeParams();

    ui->edtDefocus->setText(Utils_Qt::numToQString(p->C10 / 10)); // nm
    ui->edtSphere->setText(Utils_Qt::numToQString(p->C30 / 10000)); // um

    ui->edtStigMag->setText(Utils_Qt::numToQString(p->C12.Mag / 10)); // nm
    ui->edtStigAng->setText(Utils_Qt::numToQString((180 / Constants::Pi) * p->C12.Ang)); // degrees

    ui->edtDelta->setText(Utils_Qt::numToQString(p->Delta / 10)); // nm
    ui->edtConverge->setText(Utils_Qt::numToQString(p->Alpha)); // mrad

    ui->edtVoltage->setText(Utils_Qt::numToQString(p->Voltage)); // kV
    ui->edtAperture->setText(Utils_Qt::numToQString(p->Aperture)); // mrad
}

void AberrationFrame::updateAberrations()
{
    // here we uplaod the textbox vales to the MicroscopeParameters class

    // designed to only really be called before it matters, not after every edit...

    if (Main == nullptr)
        throw std::runtime_error("Error connecting aberration frame to main window.");
    auto params = Main->getMicroscopeParams();

    float C10 = ui->edtDefocus->text().toFloat() * 10; // Angstrom
    float C30 = ui->edtSphere->text().toFloat() * 10000; // Angstrom

    float delt = ui->edtDelta->text().toFloat() * 10; // Angstrom
    float apert = ui->edtAperture->text().toFloat(); // mrad
    float conv = ui->edtConverge->text().toFloat(); // mrad
    float volt = ui->edtVoltage->text().toFloat(); // kV

    float C12m = ui->edtStigMag->text().toFloat() * 10; // Angstrom
    float C12a = ui->edtStigAng->text().toFloat() * Constants::Pi / 180; // radians

    params->Voltage = volt;
    params->Aperture = apert;
    params->Delta = delt;
    params->Alpha = conv;

    params->C10 = C10;
    params->C12 = ComplexAberration(C12m, C12a);
    params->C30 = C30;
}