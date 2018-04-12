#include <QtGui/QRegExpValidator>
#include "globalsettingsframe.h"
#include "ui_globalsettingsframe.h"

GlobalSettingsFrame::GlobalSettingsFrame(QWidget *parent, std::shared_ptr<SimulationManager> simManager) :
    QWidget(parent), Manager(simManager),
    ui(new Ui::GlobalSettingsFrame) {
    ui->setupUi(this);

    connect(parent, SIGNAL(okSignal()), this, SLOT(dlgOk_clicked()));
    connect(parent, SIGNAL(cancelSignal()), this, SLOT(dlgCancel_clicked()));
    connect(parent, SIGNAL(applySignal()), this, SLOT(dlgApply_clicked()));

    QRegExpValidator* pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));

    ui->edt3dIntegrals->setValidator(pIntValidator);
    ui->edtParallelPx->setValidator(pIntValidator);

    int three_d_int = simManager->getFull3dInts();
    int num_parallel = simManager->getParallelPixels();

    ui->edt3dIntegrals->setText(QString::number(three_d_int));
    ui->edtParallelPx->setText(QString::number(num_parallel));

    // make the label widths the same so they line up
    auto w1 = ui->lbl3d->width();
    auto w2 = ui->lblParallel->width();
    auto w3 = ui->lblParameters->width();
    auto w = std::max(std::max(w1, w2), w3);
    ui->lbl3d->setMinimumWidth(w);
    ui->lblParallel->setMinimumWidth(w);

    ui->tabWidget->tabBar()->hide();

    populateParamsCombo();
}

GlobalSettingsFrame::~GlobalSettingsFrame()
{
    delete ui;
}

void GlobalSettingsFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    parentWidget()->close();
}

void GlobalSettingsFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    dlgApply_clicked();
    parentWidget()->close();
}

void GlobalSettingsFrame::dlgApply_clicked()
{
    int n_3d = ui->edt3dIntegrals->text().toInt();
    int n_parallel = ui->edtParallelPx->text().toInt();

    Manager->setFull3dInts(n_3d);
    Manager->setParallelPixels(n_parallel);
}

void GlobalSettingsFrame::populateParamsCombo() {
    auto names = StructureParameters::getNames();

    for (int i = 0; i < names.size(); ++i)
        ui->cmbParams->addItem(QString::fromStdString(names[i]));

    auto cur = StructureParameters::getCurrent();
    ui->cmbParams->setCurrentIndex(cur);
}

void GlobalSettingsFrame::on_cmbParams_currentIndexChanged(int index) {
    std::string text = ui->cmbParams->currentText().toStdString();


}
