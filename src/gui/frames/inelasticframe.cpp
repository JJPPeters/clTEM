#include "inelasticframe.h"
#include "ui_inelasticframe.h"

#include <utilities/stringutils.h>
#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include <controls/editunitsbox.h>

#include "dialogs/settings/settingsdialog.h"

InelasticFrame::InelasticFrame(QWidget *parent) :
    QWidget(parent), Main(nullptr),
    ui(new Ui::InelasticFrame)
{
    ui->setupUi(this);

    auto pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));
    auto pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    connect(ui->edtIterations, &QLineEdit::textChanged, this, &InelasticFrame::checkEditZero);

    ui->edtIterations->setValidator(pIntValidator);
    ui->edtPlasmonSingle->setValidator(pIntValidator);
    ui->edtPhononDefault->setValidator(pmValidator);

    ui->edtPhononDefault->setUnits("Å²");
}

InelasticFrame::~InelasticFrame()
{
    delete ui;
}

void InelasticFrame::checkEditZero(QString dud)
{
    (void)dud; // we don't use this

    auto * edt = dynamic_cast<EditUnitsBox*>(sender());

    if(edt == nullptr)
        return;

    auto t = edt->text().toStdString();
    double val = edt->text().toDouble();

    if (val <= 0)
        edt->setStyleSheet("color: #FF8C00"); // I just chose orange, mgiht want to be a better colour
    else
        edt->setStyleSheet("");
}

void InelasticFrame::on_btnPhononMore_clicked() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    // here we update the current manager from the text boxes here so the dialog can show the same
    updateManager();

    auto myDialog = new ThermalScatteringDialog(this, Main->Manager);
    connect(myDialog, &ThermalScatteringDialog::appliedSignal, this, &InelasticFrame::updatePhononsGui);
    myDialog->exec();
}

void InelasticFrame::on_btnPlasmonMore_clicked() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    // here we update the current manager from the text boxes here so the dialog can show the same
    updateManager();

    auto myDialog = new PlasmonDialog(this, Main->Manager);
    connect(myDialog, &PlasmonDialog::appliedSignal, this, &InelasticFrame::updatePlasmonsGui);
    myDialog->exec();
}

void InelasticFrame::updatePhononsGui() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    auto phonon = Main->Manager->inelasticScattering()->phonons();

    bool enabled = phonon->getFrozenPhononEnabled();
    double def_u = phonon->getDefault();

    ui->edtPhononDefault->setText(QString::number(def_u));
    ui->chkPhonon->setChecked(enabled);
}

void InelasticFrame::updatePlasmonsGui() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    auto plasmon = Main->Manager->inelasticScattering()->plasmons();

    bool enabled = plasmon->enabled();
    bool full_enabled = plasmon->simType() == PlasmonType::Full;
    bool single_enabled = !full_enabled;

    unsigned int single = plasmon->individualPlasmon();

    ui->chkPlasmon->setChecked(enabled);
    ui->rdioPlasmonAll->setChecked(full_enabled);
    ui->rdioPlasmonSingle->setChecked(single_enabled);
    ui->edtPlasmonSingle->setText(QString::number(single));
}

void InelasticFrame::updateIterationsGui() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    unsigned int its = Main->Manager->inelasticScattering()->storedIterations();

    ui->edtIterations->setText(QString::number(its));
}

void InelasticFrame::updatePhononsManager() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    double def_u = ui->edtPhononDefault->text().toDouble();
    bool enabled = ui->chkPhonon->isChecked();
    bool force_default = ui->chkPhononDefault->isChecked();

    auto phonon = Main->Manager->inelasticScattering()->phonons();

    phonon->setFrozenPhononEnabled(enabled);
    phonon->setDefault(def_u);
    phonon->force_default = force_default;
}

void InelasticFrame::updatePlasmonsManager() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    bool enabled = ui->chkPlasmon->isChecked();
    bool do_full = ui->rdioPlasmonAll->isChecked();
    bool do_single = ui->rdioPlasmonSingle->isChecked();
    unsigned int single = ui->edtPlasmonSingle->text().toUInt();

    auto plasmon = Main->Manager->inelasticScattering()->plasmons();

    plasmon->setEnabled(enabled);
    if (do_full)
        plasmon->setSimType(PlasmonType::Full);
    else if (do_single)
        plasmon->setSimType(PlasmonType::Individual);
    plasmon->setIndividualPlasmon(single);
}

void InelasticFrame::updateIterationsManager() {
    if (Main == nullptr)
        throw std::runtime_error("Error connecting inelastic frame to main window.");

    unsigned int in_it = ui->edtIterations->text().toUInt();

    Main->Manager->inelasticScattering()->setIterations(in_it);
}