#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
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




AberrationsDialog::AberrationsDialog(QWidget *parent, std::shared_ptr<MicroscopeParameters> params) :
    SettingsDialog(parent)
{
    AberrFrame = new FullAberrationFrame(this, params);
    ui->vLayout->insertWidget(0, AberrFrame);

    this->setWindowTitle("Aberrations");

    // don't think I ever need to disconnect this, should be handles by Qt when the objects are destroyed
    connect(AberrFrame, SIGNAL(aberrationsApplied()), this, SLOT(coreAberrationsChanged()));

    this->setFixedSize(this->minimumSizeHint());
}

void AberrationsDialog::coreAberrationsChanged()
{
    emit aberrationsChanged();
}




SimAreaDialog::SimAreaDialog(QWidget *parent, std::shared_ptr<SimulationArea> simArea, std::shared_ptr<CrystalStructure> structure) :
        SettingsDialog(parent)
{
    AreaFrame = new SimAreaFrame(this, simArea, structure);
    ui->vLayout->insertWidget(0, AreaFrame);

    this->setWindowTitle("Simulation area");

    connect(AreaFrame, SIGNAL(areaChanged()), this, SLOT(coreSimAreaChanged()));

    this->setFixedSize(this->minimumSizeHint());;
}

void SimAreaDialog::coreSimAreaChanged()
{
    emit simAreaChanged();
}


StemDetectorDialog::StemDetectorDialog(QWidget *parent, std::vector<StemDetector>& dets) : //, std::vector<std::shared_ptr<StemDetector>> detectors) :
        SettingsDialog(parent)
{
    DetFrame = new StemDetectorFrame(this, dets);//, simArea);
    ui->vLayout->insertWidget(0, DetFrame);

    this->setWindowTitle("STEM detectors");

    connect(DetFrame, SIGNAL(detectorsChanged()), this, SLOT(coreDetectorsChanged()));

    this->setFixedSize(this->minimumSizeHint());;
}

void StemDetectorDialog::coreDetectorsChanged()
{
    emit detectorsChanged();
}



void StemAreaDialog::coreStemAreaChanged()
{
    emit stemAreaChanged();
}

StemAreaDialog::StemAreaDialog(QWidget *parent, std::shared_ptr<StemArea> area, std::shared_ptr<SimulationArea> sim)
{
    AreaFrame = new StemAreaFrame(this, area, sim);
    ui->vLayout->insertWidget(0, AreaFrame);

    this->setWindowTitle("STEM area");

    connect(AreaFrame, SIGNAL(areaChanged()), this, SLOT(coreStemAreaChanged()));

    this->setFixedSize(this->minimumSizeHint());;
}
