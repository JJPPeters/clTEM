#include "sourceframe.h"
#include "ui_sourceframe.h"

#include <utilities/stringutils.h>
#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include <controls/editunitsbox.h>

#include "dialogs/settings/settingsdialog.h"
#include "aberrationframe.h"

SourceFrame::SourceFrame(QWidget *parent) :
    QWidget(parent), ui(new Ui::SourceFrame), Main(nullptr)
{
    ui->setupUi(this);

    auto* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));
    auto* pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtVoltage->setValidator(pValidator);

    ui->edtConAper->setValidator(pValidator);
    ui->edtConAperSig->setValidator(pValidator);

    ui->edtObjAper->setValidator(pValidator);
    ui->edtObjAperSig->setValidator(pValidator);

    ui->edtBeamTilt->setValidator(pmValidator);
    ui->edtBeamAzimuth->setValidator(pmValidator);

    ui->edtVoltage->setUnits("kV");

    ui->edtConAper->setUnits("mrad");
    ui->edtConAperSig->setUnits("mrad");

    ui->edtObjAper->setUnits("mrad");
    ui->edtObjAperSig->setUnits("mrad");

    ui->edtBeamTilt->setUnits("mrad");
    ui->edtBeamAzimuth->setUnits("°");

    // these connect to a slot that makes the text colour change if the value is zero
    // I don't want to disable 0 in the regex as people might want it as a leading character (e.g. 0.1)
    connect(ui->edtVoltage, &QLineEdit::textChanged, this, &SourceFrame::checkEditZero); // this also has a 'by name' slot

    connect(ui->edtConAper, &QLineEdit::textChanged, this, &SourceFrame::checkEditZero);

    connect(ui->edtObjAper, &QLineEdit::textChanged, this, &SourceFrame::checkEditZero);
}

SourceFrame::~SourceFrame()
{
    delete ui;
}

void SourceFrame::checkEditZero(QString dud)
{
    (void)dud; // we don't use this

    auto * edt = dynamic_cast<EditUnitsBox*>(sender());

    if(edt == nullptr)
        return;

    auto t = edt->text().toStdString();
    double val = edt->text().toDouble();

    if (val <= 0)
        edt->setForegroundStyle("; color: #FF8C00"); // I just chose orange, mgiht want to be a better colour
    else
        edt->setForegroundStyle("");
}

void SourceFrame::on_btnMore_clicked()
{
    if (Main == nullptr)
        throw std::runtime_error("Error connecting microscope frame to main window.");

    // here we update the current aberrations from the text boxes here so the dialog can show the same
    // this will also update from this frame
    Main->updateAberrationManager();

    AberrationsDialog* myDialog = new AberrationsDialog(nullptr, Main->Manager);

    // how this is disconnected when the dialog is destroyed...
    connect(myDialog, &AberrationsDialog::appliedSignal, Main, &MainWindow::updateAberrationBoxes);

    // we don't need to try and get anything from this dialog as it just updates the pointers we gave it!
    myDialog->exec();
}

void SourceFrame::updateTextBoxes()
{
    if (Main == nullptr)
        throw std::runtime_error("Error connecting microscope frame to main window.");
    auto p = Main->Manager->microscopeParams();

    ui->edtVoltage->setText(Utils_Qt::numToQString(p->Voltage)); // kV

    ui->edtConAper->setText(Utils_Qt::numToQString(p->CondenserAperture)); // mrad
    ui->edtConAperSig->setText(Utils_Qt::numToQString(p->CondenserApertureSmoothing)); // mrad

    ui->edtObjAper->setText(Utils_Qt::numToQString(p->ObjectiveAperture)); // mrad
    ui->edtObjAperSig->setText(Utils_Qt::numToQString(p->ObjectiveApertureSmoothing)); // mrad

    ui->edtBeamTilt->setText(Utils_Qt::numToQString(p->BeamTilt)); // mrad
    ui->edtBeamAzimuth->setText(Utils_Qt::numToQString((180 / Constants::Pi) * p->BeamAzimuth)); // degrees
}

void SourceFrame::updateManagerFromGui()
{
    // here we upload the textbox vales to the MicroscopeParameters class
    // designed to only really be called before it matters, not after every edit...

    if (Main == nullptr)
        throw std::runtime_error("Error connecting microscope frame to main window.");

    auto params = Main->Manager->microscopeParams();

    double volt = ui->edtVoltage->text().toDouble(); // kV

    double con_ap = ui->edtConAper->text().toDouble(); // mrad
    double con_ap_sig = ui->edtConAperSig->text().toDouble(); // mrad

    double obj_ap = ui->edtObjAper->text().toDouble(); // mrad
    double obj_ap_sig = ui->edtObjAperSig->text().toDouble(); // mrad

    double beam_tilt = ui->edtBeamTilt->text().toDouble(); // mrad
    double beam_azimuth = ui->edtBeamAzimuth->text().toDouble() * Constants::Pi / 180; // radians

    params->Voltage = volt;
    params->CondenserAperture = con_ap;
    params->CondenserApertureSmoothing = con_ap_sig;

    params->ObjectiveAperture = obj_ap;
    params->ObjectiveApertureSmoothing = obj_ap_sig;

    params->BeamTilt = beam_tilt;
    params->BeamAzimuth = beam_azimuth;
}

void SourceFrame::on_edtVoltage_textChanged(const QString &arg1) {
    // this slot is used to update the other panels for the mrad scale change
    if (Main == nullptr)
        throw std::runtime_error("Error connecting microscope frame to main window.");

    double volt = ui->edtVoltage->text().toDouble();
    Main->updateVoltageMrad( volt );
}

void SourceFrame::setModeStyles(SimulationMode md, bool tem_image) {
    QColor disabled_col = qApp->palette().color(QPalette::Disabled, QPalette::Base);
    std::string disabled_hex = disabled_col.name().toStdString();

    std::string disabled_Default = "background-color: " + disabled_hex;

    if (md == SimulationMode::CTEM) {
        ui->edtConAper->setBackgroundStyle(disabled_Default);
        ui->edtConAperSig->setBackgroundStyle(disabled_Default);

        if (tem_image) {
            ui->edtObjAper->setBackgroundStyle("");
            ui->edtObjAperSig->setBackgroundStyle("");
        } else {
            ui->edtObjAper->setBackgroundStyle(disabled_Default);
            ui->edtObjAperSig->setBackgroundStyle(disabled_Default);
        }
    } else {
        ui->edtObjAper->setBackgroundStyle(disabled_Default);
        ui->edtObjAperSig->setBackgroundStyle(disabled_Default);

        ui->edtConAper->setBackgroundStyle("");
        ui->edtConAperSig->setBackgroundStyle("");
    }

}
