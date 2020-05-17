#include <dialogs/settings/settingsdialog.h>
#include <utilities/stringutils.h>
#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include "stemframe.h"
#include "ui_stemframe.h"

StemFrame::StemFrame(QWidget *parent) :
    QWidget(parent), Main(0),
    ui(new Ui::StemFrame)
{
    ui->setupUi(this);

    QRegExpValidator* pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));

    ui->edtTds->setValidator(pIntValidator);

    connect(ui->edtTds, &QLineEdit::textChanged, this, &StemFrame::edtTds_changed);
}

StemFrame::~StemFrame()
{
    delete ui;
}

void StemFrame::on_btnDetectors_clicked()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting STEM frame to main window.");
    StemDetectorDialog* myDialog = new StemDetectorDialog(nullptr, Main->getDetectors());

    connect(myDialog, &StemDetectorDialog::detectorsChanged, this, &StemFrame::updateDetectors);

    myDialog->exec();
}

void StemFrame::updateDetectors()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting STEM frame to main window.");
    Main->setDetectors();
}

void StemFrame::on_btnArea_clicked()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting simulation frame to main window.");

    Main->updateManagerFromGui();

    SimAreaDialog* myDialog = new SimAreaDialog(nullptr, Main->Manager);

    connect(myDialog->getFrame(), &AreaLayoutFrame::resolutionChanged, Main->getSimulationFrame(), &SimulationFrame::setResolutionText);
    connect(myDialog->getFrame(), &AreaLayoutFrame::modeChanged, Main, &MainWindow::set_active_mode);
    connect(myDialog->getFrame(), &AreaLayoutFrame::updateMainCbed, Main->getCbedFrame(), &CbedFrame::updateTextBoxes);
    connect(myDialog->getFrame(), &AreaLayoutFrame::updateMainStem, this, &StemFrame::updateScaleLabels);
    connect(myDialog->getFrame(), &AreaLayoutFrame::areaChanged, Main, &MainWindow::updateScales);

    myDialog->exec();

    Main->updateScales();
}

void StemFrame::updateScaleLabels()
{
    if (Main == nullptr)
        throw std::runtime_error("Error connecting STEM frame to main window.");

    double scaleX = Main->getStemArea()->getScaleX();
    double scaleY = Main->getStemArea()->getScaleY();

    ui->lblStemScaleX->setText( "x: " + Utils_Qt::numToQString(scaleX, 2) + " Å" );
    ui->lblStemScaleY->setText( "y: " + Utils_Qt::numToQString(scaleY, 2) + " Å" );
}

void StemFrame::edtTds_changed_proxy(const QString &arg1, bool update_partner) {
    if(arg1.toInt() < 1)
        ui->edtTds->setStyleSheet("color: #FF8C00");
    else
        ui->edtTds->setStyleSheet("");

    if (update_partner)
        Main->getCbedFrame()->setTdsRuns(arg1.toUInt());
}

void StemFrame::edtTds_changed(const QString &arg1)
{
    edtTds_changed_proxy(arg1, true);
}

void StemFrame::setTdsRuns(unsigned int runs) {
    disconnect(ui->edtTds, &QLineEdit::textChanged, this, &StemFrame::edtTds_changed);
    auto new_num = QString::number(runs);
    ui->edtTds->setText(new_num);
    edtTds_changed_proxy(new_num, false);
    connect(ui->edtTds, &QLineEdit::textChanged, this, &StemFrame::edtTds_changed);
}

void StemFrame::on_btnSim_clicked()
{
    emit startSim();
}

void StemFrame::on_chkTds_stateChanged(int state)
{
    // this just updates the other frame to have the same state
    Main->getCbedFrame()->setTdsEnabled(state != 0);
}

void StemFrame::setTdsEnabled(bool enabled)
{
    ui->chkTds->setChecked(enabled);
}

bool StemFrame::isTdsEnabled()
{
    return ui->chkTds->checkState() == Qt::Checked;
}

unsigned int StemFrame::getTdsRuns()
{
    return ui->edtTds->text().toUInt();
}

void StemFrame::setActive(bool active)
{
    ui->btnSim->setEnabled(active);
}

void StemFrame::on_btnCancel_clicked()
{
    emit stopSim();
}

void StemFrame::updateTextBoxes() {
    if (Main == 0)
        throw std::runtime_error("Error connecting STEM frame to main window.");

    ui->edtTds->setText(Utils_Qt::numToQString(Main->Manager->getInelasticScattering()->getStoredInelasticIterations()));
}

void StemFrame::updateTds() {
    ui->chkTds->setChecked( Main->Manager->getInelasticScattering()->getPhonons()->getFrozenPhononEnabled() );
}
