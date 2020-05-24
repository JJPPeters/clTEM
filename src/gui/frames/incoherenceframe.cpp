#include "incoherenceframe.h"
#include "ui_incoherenceframe.h"

#include <utilities/stringutils.h>
#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include <controls/editunitsbox.h>

#include "dialogs/settings/settingsdialog.h"

IncoherenceFrame::IncoherenceFrame(QWidget *parent) :
    QWidget(parent), Main(nullptr),
    ui(new Ui::IncoherenceFrame)
{
    ui->setupUi(this);

    auto pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));
//    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtDelta->setValidator(pValidator);
    ui->edtAlpha->setValidator(pValidator);

//    ui->edtDefocus->setValidator(pmValidator);
//    ui->edtSphere->setValidator(pmValidator);
//    ui->edtStigMag->setValidator(pmValidator);
//    ui->edtStigAng->setValidator(pmValidator);
//
//    ui->edtAperture->setValidator(pValidator);
//    ui->edtDelta->setValidator(pValidator);
//    ui->edtConverge->setValidator(pValidator);
//    ui->edtVoltage->setValidator(pValidator);
//
//    ui->edtBeamTilt->setValidator(pmValidator);
//    ui->edtBeamAzimuth->setValidator(pmValidator);

    ui->edtDelta->setUnits("nm");
    ui->edtAlpha->setUnits("mrad");

//    ui->edtDefocus->setUnits("nm");
//    ui->edtSphere->setUnits("μm");
//    ui->edtStigMag->setUnits("nm");
//    ui->edtStigAng->setUnits("°");
//
//    ui->edtAperture->setUnits("mrad");
//    ui->edtDelta->setUnits("nm");
//    ui->edtConverge->setUnits("mrad");
//    ui->edtVoltage->setUnits("kV");
//
//    ui->edtBeamTilt->setUnits("mrad");
//    ui->edtBeamAzimuth->setUnits("°");
//
//    // these connect to a slot that makes the text colour change if the value is zero
//    // I don't want to disable 0 in the regex as people might want it as a leading character (e.g. 0.1)
//    connect(ui->edtAperture, &QLineEdit::textChanged, this, &IncoherenceFrame::checkEditZero);
//    connect(ui->edtDelta, &QLineEdit::textChanged, this, &IncoherenceFrame::checkEditZero);
//    connect(ui->edtConverge, &QLineEdit::textChanged, this, &IncoherenceFrame::checkEditZero);
//    connect(ui->edtVoltage, &QLineEdit::textChanged, this, &IncoherenceFrame::checkEditZero); // this also has a 'by name' slot
}

IncoherenceFrame::~IncoherenceFrame()
{
    delete ui;
}

void IncoherenceFrame::updateTemTextBoxes() {
    ui->edtDelta->setText(QString::number(Main->Manager->microscopeParams()->Delta / 10.0)); // to nm
    ui->edtAlpha->setText(QString::number(Main->Manager->microscopeParams()->Alpha )); // mrad
}

void IncoherenceFrame::updateTemManager() {
    double delta = ui->edtDelta->text().toDouble();
    double alpha = ui->edtAlpha->text().toDouble();

    Main->Manager->microscopeParams()->Delta = delta * 10; // nm to A
    Main->Manager->microscopeParams()->Alpha = alpha; // rad
}

//void IncoherenceFrame::checkEditZero(QString dud)
//{
//    (void)dud; // we don't use this
//
//    auto * edt = dynamic_cast<EditUnitsBox*>(sender());
//
//    if(edt == nullptr)
//        return;
//
//    auto t = edt->text().toStdString();
//    double val = edt->text().toDouble();
//
//    if (val <= 0)
//        edt->setStyleSheet("color: #FF8C00"); // I just chose orange, mgiht want to be a better colour
//    else
//        edt->setStyleSheet("");
//}

//void IncoherenceFrame::updateTextBoxes()
//{
//    if (Main == nullptr)
//        throw std::runtime_error("Error connecting aberration frame to main window.");
//    auto p = Main->Manager->microscopeParams();
//
//    ui->edtDefocus->setText(Utils_Qt::numToQString(p->C10 / 10)); // nm
//    ui->edtSphere->setText(Utils_Qt::numToQString(p->C30 / 10000)); // um
//
//    ui->edtStigMag->setText(Utils_Qt::numToQString(p->C12.Mag / 10)); // nm
//    ui->edtStigAng->setText(Utils_Qt::numToQString((180 / Constants::Pi) * p->C12.Ang)); // degrees
//
//    ui->edtDelta->setText(Utils_Qt::numToQString(p->Delta / 10)); // nm
//    ui->edtConverge->setText(Utils_Qt::numToQString(p->Alpha)); // mrad
//
//    ui->edtVoltage->setText(Utils_Qt::numToQString(p->Voltage)); // kV
//    ui->edtAperture->setText(Utils_Qt::numToQString(p->Aperture)); // mrad
//
//    ui->edtBeamTilt->setText(Utils_Qt::numToQString(p->BeamTilt)); // mrad
//    ui->edtBeamAzimuth->setText(Utils_Qt::numToQString((180 / Constants::Pi) * p->BeamAzimuth)); // degrees
//}

//void IncoherenceFrame::updateAberrations()
//{
//    // here we uplaod the textbox vales to the MicroscopeParameters class
//
//    // designed to only really be called before it matters, not after every edit...
//
//    if (Main == nullptr)
//        throw std::runtime_error("Error connecting aberration frame to main window.");
//    auto params = Main->Manager->microscopeParams();
//
//    auto test = ui->edtDefocus->text().toStdString();
//    double C10 = ui->edtDefocus->text().toDouble() * 10; // Angstrom
//    double C30 = ui->edtSphere->text().toDouble() * 10000; // Angstrom
//
//    double delt = ui->edtDelta->text().toDouble() * 10; // Angstrom
//    double apert = ui->edtAperture->text().toDouble(); // mrad
//    double conv = ui->edtConverge->text().toDouble(); // mrad
//    double volt = ui->edtVoltage->text().toDouble(); // kV
//
//    double C12m = ui->edtStigMag->text().toDouble() * 10; // Angstrom
//    double C12a = ui->edtStigAng->text().toDouble() * Constants::Pi / 180; // radians
//
//    double beam_tilt = ui->edtBeamTilt->text().toDouble(); // mrad
//    double beam_azimuth = ui->edtBeamAzimuth->text().toDouble() * Constants::Pi / 180; // radians
//
//    params->Voltage = volt;
//    params->Aperture = apert;
//    params->Delta = delt;
//    params->Alpha = conv;
//
//    params->C10 = C10;
//    params->C12 = ComplexAberration(C12m, C12a);
//    params->C30 = C30;
//
//    params->BeamTilt = beam_tilt;
//    params->BeamAzimuth = beam_azimuth;
//}
//
//void IncoherenceFrame::on_edtVoltage_textChanged(const QString &arg1) {
//    // this slot is used to update the other panels for the mrad scale change
//    if (Main == nullptr)
//        throw std::runtime_error("Error connecting aberration frame to main window.");
//
//    double volt = ui->edtVoltage->text().toDouble();
//    Main->updateVoltageMrad( volt );
//}
