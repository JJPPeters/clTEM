#include <QtGui/QRegExpValidator>
#include <utility>
#include <dialogs/settings/settingsdialog.h>
#include "globalsimsettingsframe.h"
#include "ui_globalsimsettingsframe.h"

GlobalSimSettingsFrame::GlobalSimSettingsFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager) :
    QWidget(parent), ui(new Ui::GlobalSimSettingsFrame), Manager(std::move(simManager))
{
    ui->setupUi(this);

    auto parent_dlg = dynamic_cast<GlobalSettingsDialog*>(parentWidget());
    connect(parent_dlg, &GlobalSettingsDialog::okSignal, this, &GlobalSimSettingsFrame::dlgOk_clicked);
    connect(parent_dlg, &GlobalSettingsDialog::cancelSignal, this, &GlobalSimSettingsFrame::dlgCancel_clicked);
    connect(parent_dlg, &GlobalSettingsDialog::applySignal, this, &GlobalSimSettingsFrame::dlgApply_clicked);

    ui->edtPaddingXY->setUnits("Å");
    ui->edtPaddingZ->setUnits("Å");

    auto* pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));
    auto* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edt3dIntegrals->setValidator(pIntValidator);
    ui->edtParallelPx->setValidator(pIntValidator);
    ui->edtMixPot->setValidator(pIntValidator);

    ui->edtPaddingXY->setValidator(pValidator);
    ui->edtPaddingZ->setValidator(pValidator);

    unsigned int three_d_int = Manager->full3dIntegrals();
    unsigned int num_parallel = Manager->storedParallelPixels();

    ui->chkDoublePrec->setChecked(Manager->doublePrecisionEnabled());
    ui->chkParallelStem->setChecked(Manager->parallelStem());
    ui->chkPrecalcTrans->setChecked(Manager->precalculateTransmission());

    ui->edt3dIntegrals->setText(QString::number(three_d_int));
    ui->edtParallelPx->setText(QString::number(num_parallel));

    ui->edtPaddingXY->setText(QString::number(Manager->simulationCell()->defaultPaddingXY()[1]));
    ui->edtPaddingZ->setText(QString::number(Manager->simulationCell()->defaultPaddingZ()[1]));

    ui->edtMixPot->setText(QString::number(Manager->storedParallelPotentialsCount()));
    ui->chkMixPot->setChecked(Manager->storedUseParallelPotentials());

    ui->chkForceResort->setChecked(Manager->forcePhononAtomResort());

    // make the label widths the same so they line up
    auto w1 = ui->lbl3d->width();
    auto w2 = ui->lblParallel->width();
    auto w3 = ui->lblParameters->width();
    auto w4 = ui->lblPaddingXY->width();
    auto w5 = ui->lblPaddingZ->width();
    auto w6 = ui->lblUseMixPot->width();
    auto w = std::max(std::max(w1, w2), std::max(w3, std::max(w4, std::max(w5, w6))));
    ui->lbl3d->setMinimumWidth(w);
    ui->lblParallel->setMinimumWidth(w);
    ui->lblParameters->setMinimumWidth(w);
    ui->lblPaddingXY->setMinimumWidth(w);
    ui->lblPaddingZ->setMinimumWidth(w);
    ui->lblUseMixPot->setMinimumWidth(w);
    // TODO: do this for all labels

    populateParamsCombo();

    connect(ui->edt3dIntegrals, &QLineEdit::textChanged, this, &GlobalSimSettingsFrame::checkValidInputs);
    connect(ui->edtParallelPx, &QLineEdit::textChanged, this, &GlobalSimSettingsFrame::checkValidInputs);
    connect(ui->edtMixPot, &QLineEdit::textChanged, this, &GlobalSimSettingsFrame::checkValidInputs);

}

GlobalSimSettingsFrame::~GlobalSimSettingsFrame() {
    delete ui;
}

void GlobalSimSettingsFrame::checkValidInputs() {
//    bool valid = true;

    if (ui->edt3dIntegrals->text().toInt() > 0)
        ui->edt3dIntegrals->setStyleSheet("");
    else {
        ui->edt3dIntegrals->setStyleSheet("color: #FF8C00");
//        valid = false;
    }

    if (ui->edtParallelPx->text().toInt() > 0)
        ui->edtParallelPx->setStyleSheet("");
    else {
        ui->edtParallelPx->setStyleSheet("color: #FF8C00");
//        valid = false;
    }

    // don't do anything with valid right now, but I could disable the  apply button?
}

void GlobalSimSettingsFrame::dlgCancel_clicked() {
    // don't need to do anything, just return
    parentWidget()->close();
}

void GlobalSimSettingsFrame::dlgOk_clicked() {
    // same as clicking apply then closing the dialog
    dlgApply_clicked();
    parentWidget()->close();
}

void GlobalSimSettingsFrame::dlgApply_clicked() {
    unsigned int n_3d = ui->edt3dIntegrals->text().toUInt();
    unsigned int n_parallel = ui->edtParallelPx->text().toUInt();
    std::string param_name = ui->cmbParams->currentText().toStdString();
    double pad_xy = ui->edtPaddingXY->text().toDouble();
    double pad_z = ui->edtPaddingZ->text().toDouble();
    bool do_double = ui->chkDoublePrec->isChecked();
    bool precalc = ui->chkPrecalcTrans->isChecked();
    bool prlll = ui->chkParallelStem->isChecked();
    unsigned int n_mp = ui->edtMixPot->text().toUInt();
    bool use_mp = ui->chkMixPot->isChecked();
    bool force_resort = ui->chkForceResort->isChecked();

    Manager->setFull3dIntegrals(n_3d);
    Manager->setParallelPixels(n_parallel);
    Manager->setStructureParameters(param_name);
    Manager->simulationCell()->setDefaultPaddingXY({-pad_xy, pad_xy});
    Manager->simulationCell()->setDefaultPaddingZ({-pad_z, pad_z});
    Manager->setDoublePrecisionEnabled(do_double);
    Manager->setPrecalculateTransmission(precalc);
    Manager->setParallelStem(prlll);
    Manager->setParallelPotentialsCount(n_mp);
    Manager->setUseParallelPotentials(use_mp);
    Manager->setForcePhononAtomResort(force_resort);

    emit dynamic_cast<GlobalSettingsDialog*>(parentWidget())->appliedSignal();
}

void GlobalSimSettingsFrame::populateParamsCombo() {
    auto names = StructureParameters::getNames();

    auto cur = Manager->structureParameters().name;

    unsigned int current = 0;
    for (unsigned int i = 0; i < names.size(); ++i) {
        if (names[i] == cur)
            current = i;
        ui->cmbParams->addItem(QString::fromStdString(names[i]));
    }

    ui->cmbParams->setCurrentIndex((int) current);
}