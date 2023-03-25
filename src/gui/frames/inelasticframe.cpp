#include "inelasticframe.h"
#include "ui_inelasticframe.h"

#include <utilities/stringutils.h>
#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include <controls/editunitsbox.h>

#include "dialogs/settings/settingsdialog.h"

InelasticFrame::InelasticFrame(QWidget *parent) :
    QWidget(parent), ui(new Ui::InelasticFrame), Main(nullptr)
{
    ui->setupUi(this);

    auto pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));
    auto pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtPlasmonSingle->setValidator(pIntValidator);
    ui->edtPhononDefault->setValidator(pmValidator);

    ui->edtPhononDefault->setUnits("Å²");

    connect(ui->chkPhonon, &QCheckBox::stateChanged, this, &InelasticFrame::checkStatesChanged);
    connect(ui->chkPlasmon, &QCheckBox::stateChanged, this, &InelasticFrame::checkStatesChanged);
}

InelasticFrame::~InelasticFrame()
{
    delete ui;
}

void InelasticFrame::on_btnPhononMore_clicked() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    // here we update the current manager from the text boxes here so the dialog can show the same
    updateManager();

    auto myDialog = new ThermalScatteringDialog(Main, Main->Manager);
    connect(myDialog, &ThermalScatteringDialog::appliedSignal, this, &InelasticFrame::updatePhononsGui);
    myDialog->exec();
}

void InelasticFrame::on_btnPlasmonMore_clicked() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    // here we update the current manager from the text boxes here so the dialog can show the same
    updateManager();

    auto myDialog = new PlasmonDialog(Main, Main->Manager);
    connect(myDialog, &PlasmonDialog::appliedSignal, this, &InelasticFrame::updatePlasmonsGui);
    myDialog->exec();
}

void InelasticFrame::updatePhononsGui() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    auto phonon = Main->Manager->incoherenceEffects()->phonons();

    bool enabled = phonon->getFrozenPhononEnabled();
    double def_u = phonon->getDefault();
    bool use_def = phonon->forceDefault();

    ui->edtPhononDefault->setText(QString::number(def_u));
    ui->chkPhononDefault->setChecked(use_def);
    ui->chkPhonon->setChecked(enabled);
}

void InelasticFrame::updatePlasmonsGui() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    auto plasmon = Main->Manager->incoherenceEffects()->plasmons();

    bool enabled = plasmon->enabled();
    bool full_enabled = plasmon->simType() == PlasmonType::Full;
    bool single_enabled = !full_enabled;

    unsigned int single = plasmon->individualPlasmon();

    ui->chkPlasmon->setChecked(enabled);
    ui->rdioPlasmonAll->setChecked(full_enabled);
    ui->rdioPlasmonSingle->setChecked(single_enabled);
    ui->edtPlasmonSingle->setText(QString::number(single));
}

void InelasticFrame::updatePhononsManager() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    double def_u = ui->edtPhononDefault->text().toDouble();
    bool enabled = ui->chkPhonon->isChecked();
    bool force_default = ui->chkPhononDefault->isChecked();

    auto phonon = Main->Manager->incoherenceEffects()->phonons();

    phonon->setFrozenPhononEnabled(enabled);
    phonon->setDefault(def_u);
    phonon->setForceDefault(force_default);
}

void InelasticFrame::updatePlasmonsManager() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    bool enabled = ui->chkPlasmon->isChecked();
    bool do_full = ui->rdioPlasmonAll->isChecked();
    bool do_single = ui->rdioPlasmonSingle->isChecked();
    unsigned int single = ui->edtPlasmonSingle->text().toUInt();

    auto plasmon = Main->Manager->incoherenceEffects()->plasmons();

    plasmon->setEnabled(enabled);
    if (do_full)
        plasmon->setSimType(PlasmonType::Full);
    else if (do_single)
        plasmon->setSimType(PlasmonType::Individual);
    plasmon->setIndividualPlasmon(single);
}