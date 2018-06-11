#include <simulationmanager.h>
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
        BorderlessDialog(parent),
        ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    addTitleBar();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_btnCancel_clicked()
{
    emit cancelSignal();
}

void SettingsDialog::on_BtnApply_clicked()
{
    emit applySignal();
}

void SettingsDialog::on_btnOk_clicked()
{
    emit okSignal();
}




OpenClDialog::OpenClDialog(QWidget *parent, std::vector<clDevice> current_devices) :
    SettingsDialog(parent)
{
    OClFrame = new OpenClFrame(this, current_devices);
    ui->vLayout->insertWidget(0, OClFrame);

    this->setWindowTitle("OpenCL");

    this->setFixedSize(this->minimumSizeHint());
}

std::tuple<std::vector<clDevice>, std::vector<float>> OpenClDialog::getChosenDevices()
{
    return OClFrame->getChosenDevices();
}




GlobalSettingsDialog::GlobalSettingsDialog(QWidget *parent, std::shared_ptr<SimulationManager> simManager) :
        SettingsDialog(parent)
{
    GeneralFrame = new GlobalSettingsFrame(this, simManager);
    ui->vLayout->insertWidget(0, GeneralFrame);

    this->setWindowTitle("General settings");

    this->setFixedSize(this->minimumSizeHint());
}


AberrationsDialog::AberrationsDialog(QWidget *parent, std::shared_ptr<MicroscopeParameters> params) :
    SettingsDialog(parent)
{
    AberrFrame = new FullAberrationFrame(this, params);
    ui->vLayout->insertWidget(0, AberrFrame);

    this->setWindowTitle("Aberrations");

    // don't think I ever need to disconnect this, should be handles by Qt when the objects are destroyed
    connect(AberrFrame, &FullAberrationFrame::aberrationsApplied, this, &AberrationsDialog::coreAberrationsChanged);

    this->setFixedSize(this->minimumSizeHint());
}

void AberrationsDialog::coreAberrationsChanged()
{
    emit aberrationsChanged();
}




SimAreaDialog::SimAreaDialog(QWidget *parent, std::shared_ptr<SimulationManager> simManager) : SettingsDialog(parent)
{
    LayoutFrame = new AreaLayoutFrame(this, simManager);
    ui->vLayout->insertWidget(0, LayoutFrame);

    this->setWindowTitle("Simulation area");

    this->setFixedSize(this->sizeHint());
}


StemDetectorDialog::StemDetectorDialog(QWidget *parent, std::vector<StemDetector>& dets) : //, std::vector<std::shared_ptr<StemDetector>> detectors) :
        SettingsDialog(parent)
{
    DetFrame = new StemDetectorFrame(this, dets);//, simArea);
    ui->vLayout->insertWidget(0, DetFrame);

    this->setWindowTitle("STEM detectors");

    connect(DetFrame, &StemDetectorFrame::detectorsChanged, this, &StemDetectorDialog::coreDetectorsChanged);

    this->setFixedSize(this->minimumSizeHint());
}

void StemDetectorDialog::coreDetectorsChanged()
{
    emit detectorsChanged();
}


ThermalScatteringDialog::ThermalScatteringDialog(QWidget *parent, std::shared_ptr<SimulationManager> simManager) :
        SettingsDialog(parent)
{
    ThermalFrame = new ThermalScatteringFrame(this, simManager);
    ui->vLayout->insertWidget(0, ThermalFrame);

    this->setWindowTitle("Thermal scattering");

    this->setFixedSize(this->minimumSizeHint());
}


void StemAreaDialog::coreStemAreaChanged()
{
    emit stemAreaChanged();
}

StemAreaDialog::StemAreaDialog(QWidget *parent, std::shared_ptr<StemArea> area, std::shared_ptr<SimulationArea> sim)
{
    this->setWindowTitle("STEM area");

    this->setFixedSize(this->minimumSizeHint());
}




ThemeDialog::ThemeDialog(QWidget *parent) : SettingsDialog(parent) {
#ifdef _WIN32
    tFrame = new ThemeFrame(this);
    ui->vLayout->insertWidget(0, tFrame);

    this->setWindowTitle("Theme");

    this->setFixedSize(this->minimumSizeHint());
#endif
}