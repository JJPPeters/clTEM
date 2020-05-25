#include "aberrationframe.h"
#include "ui_aberrationframe.h"

#include <utilities/stringutils.h>
#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include <controls/editunitsbox.h>

#include "dialogs/settings/settingsdialog.h"

AberrationFrame::AberrationFrame(QWidget *parent) :
    QWidget(parent), Main(nullptr),
    ui(new Ui::AberrationFrame)
{
    ui->setupUi(this);

    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));
    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtDefocus->setValidator(pmValidator);
    ui->edtStigMag->setValidator(pmValidator);
    ui->edtStigAng->setValidator(pmValidator);

    ui->edtComaMag->setValidator(pmValidator);
    ui->edtComaAng->setValidator(pmValidator);
    ui->edtThreeFoldMag->setValidator(pmValidator);
    ui->edtThreeFoldAng->setValidator(pmValidator);

    ui->edtSpherical->setValidator(pmValidator);
    ui->edtStarMag->setValidator(pmValidator);
    ui->edtStarAng->setValidator(pmValidator);
    ui->edtFourFoldMag->setValidator(pmValidator);
    ui->edtFourFoldAng->setValidator(pmValidator);

    ui->edtDefocus->setUnits("nm");
    ui->edtStigMag->setUnits("nm");
    ui->edtStigAng->setUnits("°");

    ui->edtComaMag->setUnits("nm");
    ui->edtComaAng->setUnits("°");
    ui->edtThreeFoldMag->setUnits("nm");
    ui->edtThreeFoldAng->setUnits("°");

    ui->edtSpherical->setUnits("μm");
    ui->edtStarMag->setUnits("μm");
    ui->edtStarAng->setUnits("°");
    ui->edtFourFoldMag->setUnits("μm");
    ui->edtFourFoldAng->setUnits("°");
}

AberrationFrame::~AberrationFrame()
{
    delete ui;
}

void AberrationFrame::on_btnMore_clicked()
{
    if (Main == nullptr)
        throw std::runtime_error("Error connecting aberration frame to main window.");

    // here we update the current aberrations from the text boxes here so the dialog can show the same
    // this will also update from this frame
    Main->updateAberrationManager();

    AberrationsDialog* myDialog = new AberrationsDialog(nullptr, Main->Manager->microscopeParams());

    // how this is dosconnected when the dialog is destroyed...
    connect(myDialog, &AberrationsDialog::appliedSignal, Main, &MainWindow::updateAberrationBoxes);

    // we don't need to try and get anything from this dialog as it just updates the pointers we gave it!
    myDialog->exec();
}

void AberrationFrame::updateTextBoxes()
{
    if (Main == nullptr)
        throw std::runtime_error("Error connecting aberration frame to main window.");
    auto p = Main->Manager->microscopeParams();

    ui->edtDefocus->setText(Utils_Qt::numToQString(p->C10 / 10)); // nm
    ui->edtStigMag->setText(Utils_Qt::numToQString(p->C12.Mag / 10)); // nm
    ui->edtStigAng->setText(Utils_Qt::numToQString((180 / Constants::Pi) * p->C12.Ang)); // degrees

    ui->edtComaMag->setText(Utils_Qt::numToQString(p->C21.Mag / 10)); // nm
    ui->edtComaAng->setText(Utils_Qt::numToQString((180 / Constants::Pi) * p->C21.Ang)); // degrees
    ui->edtThreeFoldMag->setText(Utils_Qt::numToQString(p->C23.Mag / 10)); // nm
    ui->edtThreeFoldAng->setText(Utils_Qt::numToQString((180 / Constants::Pi) * p->C23.Ang)); // degrees

    ui->edtSpherical->setText(Utils_Qt::numToQString(p->C30 / 10000)); // um
    ui->edtStarMag->setText(Utils_Qt::numToQString(p->C32.Mag / 10000)); // um
    ui->edtStarAng->setText(Utils_Qt::numToQString((180 / Constants::Pi) * p->C32.Ang)); // degrees
    ui->edtFourFoldMag->setText(Utils_Qt::numToQString(p->C34.Mag / 10000)); // um
    ui->edtFourFoldAng->setText(Utils_Qt::numToQString((180 / Constants::Pi) * p->C34.Ang)); // degrees
}

void AberrationFrame::updateAberrations()
{
    // here we uplaod the textbox vales to the MicroscopeParameters class

    // designed to only really be called before it matters, not after every edit...

    if (Main == nullptr)
        throw std::runtime_error("Error connecting aberration frame to main window.");
    auto params = Main->Manager->microscopeParams();

    auto test = ui->edtDefocus->text().toStdString();
    double C10 = ui->edtDefocus->text().toDouble() * 10; // Angstrom
    double C12m = ui->edtStigMag->text().toDouble() * 10; // Angstrom
    double C12a = ui->edtStigAng->text().toDouble() * Constants::Pi / 180; // radians

    double C21m = ui->edtComaMag->text().toDouble() * 10; // Angstrom
    double C21a = ui->edtComaAng->text().toDouble() * Constants::Pi / 180; // radians
    double C23m = ui->edtThreeFoldMag->text().toDouble() * 10; // Angstrom
    double C23a = ui->edtThreeFoldAng->text().toDouble() * Constants::Pi / 180; // radians

    double C30 = ui->edtSpherical->text().toDouble() * 10000; // Angstrom
    double C32m = ui->edtStarMag->text().toDouble() * 10000; // Angstrom
    double C32a = ui->edtStarAng->text().toDouble() * Constants::Pi / 180; // radians
    double C34m = ui->edtFourFoldMag->text().toDouble() * 10000; // Angstrom
    double C34a = ui->edtFourFoldAng->text().toDouble() * Constants::Pi / 180; // radians

    params->C10 = C10;
    params->C12 = ComplexAberration(C12m, C12a);

    params->C21 = ComplexAberration(C21m, C21a);
    params->C23 = ComplexAberration(C23m, C23a);

    params->C30 = C30;
    params->C32 = ComplexAberration(C32m, C32a);
    params->C34 = ComplexAberration(C34m, C34a);
}
