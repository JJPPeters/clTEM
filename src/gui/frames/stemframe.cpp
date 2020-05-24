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

//    QScreen* primary_screen = QGuiApplication::primaryScreen();
//    double pixel_ratio = primary_screen->devicePixelRatio();

//    int col1 = 75  / pixel_ratio;
//    int col2 = 100 / pixel_ratio;
//    int col3 = 100 / pixel_ratio;

//    auto test = dynamic_cast<QGridLayout*>(this->layout());

//    test->setColumnMinimumWidth(0, col1);
//    test->setColumnMinimumWidth(1, col2);
//    test->setColumnMinimumWidth(2, col3);
}

StemFrame::~StemFrame()
{
    delete ui;
}

void StemFrame::on_btnDetectors_clicked()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting STEM frame to main window.");
    StemDetectorDialog* myDialog = new StemDetectorDialog(nullptr, Main->Manager->stemDetectors());

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

    double scaleX = Main->Manager->stemArea()->getScaleX();
    double scaleY = Main->Manager->stemArea()->getScaleY();

    ui->lblStemScaleX->setText( "x scale: " + Utils_Qt::numToQString(scaleX, 2) + " Å" );
    ui->lblStemScaleY->setText( "y scale: " + Utils_Qt::numToQString(scaleY, 2) + " Å" );
}

void StemFrame::on_btnSim_clicked()
{
    emit startSim();
}

void StemFrame::setActive(bool active)
{
//    ui->btnSim->setEnabled(active);
}

void StemFrame::on_btnCancel_clicked()
{
    emit stopSim();
}
