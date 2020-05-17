#include <dialogs/settings/settingsdialog.h>
#ifdef _WIN32
#include <theme/thememanager.h>

#endif
#include "plasmonsettingsframe.h"
#include "ui_plasmonsettingsframe.h"
#include "utilities/logging.h"


PlasmonSettingsFrame::PlasmonSettingsFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager) :
    QWidget(parent),
    ui(new Ui::PlasmonSettingsFrame)
{
    ui->setupUi(this);

    // sort out our positive floating point text boxes
    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtMeanFreePath->setValidator(pValidator);
    ui->edtCharacteristicAngle->setValidator(pValidator);
    ui->edtCriticalAngle->setValidator(pValidator);

    ui->edtMeanFreePath->setUnits("nm");
    ui->edtCharacteristicAngle->setUnits("mrad");
    ui->edtCriticalAngle->setUnits("mrad");
    ui->edtIndividual->setUnits("th");

    // sort out our integer text box
    auto pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));

    ui->edtIndividual->setValidator(pIntValidator);

    //TODO: remove this
    plasmon_manager = simManager->getInelasticScattering()->getPlasmons();

    ui->edtMeanFreePath->setText(QString::number(plasmon_manager->getMeanFreePath() / 10)); // angstroms to nm
    ui->edtCharacteristicAngle->setText(QString::number(plasmon_manager->getCharacteristicAngle()));
    ui->edtCriticalAngle->setText(QString::number(plasmon_manager->getCriticalAngle()));

    ui->edtIndividual->setText(QString::number(plasmon_manager->getIndividualPlasmon()));

    ui->rdioCombined->setChecked(plasmon_manager->getCombinedEnabled());
    ui->rdioIndividual->setChecked(plasmon_manager->getIndividualEnabled());

    ui->chkEnabled->setChecked(plasmon_manager->getPlasmonEnabled());

    // connect up our OK, etc... buttons
    auto parent_dlg = dynamic_cast<PlasmonDialog*>(parentWidget());
    connect(parent_dlg, &PlasmonDialog::okSignal, this, &PlasmonSettingsFrame::dlgOk_clicked);
    connect(parent_dlg, &PlasmonDialog::cancelSignal, this, &PlasmonSettingsFrame::dlgCancel_clicked);
    connect(parent_dlg, &PlasmonDialog::applySignal, this, &PlasmonSettingsFrame::dlgApply_clicked);
}

PlasmonSettingsFrame::~PlasmonSettingsFrame()
{
    delete ui;
}

void PlasmonSettingsFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    parentWidget()->close();
}

void PlasmonSettingsFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    dlgApply_clicked();
    parentWidget()->close();
}

void PlasmonSettingsFrame::dlgApply_clicked()
{
    double mfp = ui->edtMeanFreePath->text().toDouble() * 10; // nm to angstroms
    double cha = ui->edtCharacteristicAngle->text().toDouble();
    double cra = ui->edtCriticalAngle->text().toDouble();

    auto individual = ui->edtIndividual->text().toUInt();

    bool do_comb = ui->rdioCombined->isChecked();
    bool do_ind = ui->rdioIndividual->isChecked();

    bool enabled = ui->chkEnabled->isChecked();

    plasmon_manager->setMeanFreePath(mfp);
    plasmon_manager->setCharacteristicAngle(cha);
    plasmon_manager->setCriticalAngle(cra);
    plasmon_manager->setIndividualPlasmon(individual);

    plasmon_manager->setCombinedEnabled(do_comb);
    plasmon_manager->setIndividualEnabled(do_ind);

    plasmon_manager->setEnabled(enabled);
}
