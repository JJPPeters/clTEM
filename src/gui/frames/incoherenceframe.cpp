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

    ui->edtCc->setValidator(pValidator);
    ui->edtDeMinus->setValidator(pValidator);
    ui->edtDePlus->setValidator(pValidator);

    ui->edtSourceSpread->setValidator(pValidator);

    ui->edtDelta->setUnits("nm");
    ui->edtAlpha->setUnits("mrad");

    ui->edtCc->setUnits("mm");
    ui->edtDeMinus->setUnits("eV");
    ui->edtDePlus->setUnits("eV");

    ui->edtSourceSpread->setUnits("Å");

    connect(ui->chkSourceSize, &QCheckBox::stateChanged, this, &IncoherenceFrame::checkStatesChanged);
    connect(ui->chkChromatic, &QCheckBox::stateChanged, this, &IncoherenceFrame::checkStatesChanged);
}

IncoherenceFrame::~IncoherenceFrame()
{
    delete ui;
}

void IncoherenceFrame::updateTemTextBoxes() {
    ui->edtDelta->setText(QString::number(Main->Manager->microscopeParams()->Delta / 10.0)); // to nm
    ui->edtAlpha->setText(QString::number(Main->Manager->microscopeParams()->Alpha )); // mrad
}

void IncoherenceFrame::updateChromaticTextBoxes() {
    auto ce = Main->Manager->incoherenceEffects()->chromatic();
    ui->edtCc->setText(QString::number(ce->chromaticAberration()));
    ui->edtDeMinus->setText(QString::number(ce->halfWidthHalfMaxNegative()));
    ui->edtDePlus->setText(QString::number(ce->halfWidthHalfMaxPositive()));

    ui->chkChromatic->setChecked(ce->enabled());
}

void IncoherenceFrame::updateSourceSizeTextBoxes() {
    auto ss = Main->Manager->incoherenceEffects()->source();
    ui->edtSourceSpread->setText(QString::number(ss->fullWidthHalfMax()));
    ui->chkSourceSize->setChecked(ss->enabled());
}

void IncoherenceFrame::updateTemManager() {
    double delta = ui->edtDelta->text().toDouble();
    double alpha = ui->edtAlpha->text().toDouble();

    Main->Manager->microscopeParams()->Delta = delta * 10; // nm to A
    Main->Manager->microscopeParams()->Alpha = alpha; // rad
}

void IncoherenceFrame::updateChromaticManager() {
    auto ce = Main->Manager->incoherenceEffects()->chromatic();

    double cc = ui->edtCc->text().toDouble();
    double de_plus = ui->edtDePlus->text().toDouble();
    double de_neg = ui->edtDeMinus->text().toDouble();

    ce->setHalfWidthHalfMaxs(de_neg, de_plus);
    ce->setChromaticAberration(cc);

    ce->setEnabled(ui->chkChromatic->isChecked());
}

void IncoherenceFrame::updateSourceSizeManager() {
    auto ss = Main->Manager->incoherenceEffects()->source();

    bool enab = ui->chkSourceSize->isChecked();
    double fwhm = ui->edtSourceSpread->text().toDouble();

    ss->setFullWidthHalfMax(fwhm);
    ss->setEnabled(enab);
}

void IncoherenceFrame::setModeStyles(SimulationMode md, bool tem_image) {
    QColor disabled_col = qApp->palette().color(QPalette::Disabled, QPalette::Base);
    std::string disabled_hex = disabled_col.name().toStdString();
    std::string disabled_Default = "background-color: " + disabled_hex;

    if (md == SimulationMode::CTEM) {
        ui->edtCc->setBackgroundStyle(disabled_Default);
        ui->edtDeMinus->setBackgroundStyle(disabled_Default);
        ui->edtDePlus->setBackgroundStyle(disabled_Default);
        ui->edtSourceSpread->setBackgroundStyle(disabled_Default);

        if (tem_image) {
            ui->edtDelta->setBackgroundStyle("");
            ui->edtAlpha->setBackgroundStyle("");
        } else {
            ui->edtDelta->setBackgroundStyle(disabled_Default);
            ui->edtAlpha->setBackgroundStyle(disabled_Default);
        }
    }
    else {
        ui->edtCc->setBackgroundStyle("");
        ui->edtDeMinus->setBackgroundStyle("");
        ui->edtDePlus->setBackgroundStyle("");
        ui->edtSourceSpread->setBackgroundStyle("");

        ui->edtDelta->setBackgroundStyle(disabled_Default);
        ui->edtAlpha->setBackgroundStyle(disabled_Default);
    }

}
