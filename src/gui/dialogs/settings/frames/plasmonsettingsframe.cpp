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

    // sort out our integer text box
    auto pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));

    ui->edtCombinedIterations->setValidator(pIntValidator);

    // sort out our number lsit text box (like a printer page select dialog)
    QRegExpValidator* iListValidator = new QRegExpValidator(QRegExp(R"([0-9,-]*)"));

    ui->edtIndividualList->setValidator(iListValidator);

    //TODO: remove this
    test = std::make_shared<Plasmons>();

    ui->edtMeanFreePath->setText(QString::number(test->getMeanFreePath()));
    ui->edtCharacteristicAngle->setText(QString::number(test->getCharacteristicAngle()));
    ui->edtCriticalAngle->setText(QString::number(test->getCriticalAngle()));

    ui->edtCombinedIterations->setText(QString::number(test->getCombinedPlasmonIterations()));
    ui->edtIndividualList->setText(QString::fromStdString(test->getSpecificPhononsAsString()));

    ui->chkCombined->setChecked(test->getSimulateCombined());
    ui->chkIndividual->setChecked(test->getSimulateIndividual());

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
    auto individuals = ui->edtIndividualList->text().toStdString();
    test->setSpecificPhononsFromString(individuals);

    double mfp = ui->edtMeanFreePath->text().toDouble();
    double cha = ui->edtCharacteristicAngle->text().toDouble();
    double cra = ui->edtCriticalAngle->text().toDouble();

    int it = ui->edtCombinedIterations->text().toInt();

    bool do_comb = ui->chkCombined->isChecked();
    bool do_ind = ui->chkIndividual->isChecked();

    test->setMeanFreePath(mfp);
    test->setCharacteristicAngle(cha);
    test->setCriticalAngle(cra);
    test->setCombinedPlasmonIterations(it);

    test->setSimulateCombined(do_comb);
    test->setSimulateIndividual(do_ind);
}
