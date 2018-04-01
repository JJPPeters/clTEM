#include <utils/stringutils.h>
#include "arealayoutframe.h"
#include "ui_arealayoutframe.h"

AreaLayoutFrame::AreaLayoutFrame(QWidget *parent, std::shared_ptr<SimulationManager> simMan) :
    QWidget(parent), SimManager(simMan),
    ui(new Ui::AreaLayoutFrame)
{
    ui->setupUi(this);

    connect(parent, SIGNAL(okSignal()), this, SLOT(dlgOk_clicked()));
    connect(parent, SIGNAL(cancelSignal()), this, SLOT(dlgCancel_clicked()));
    connect(parent, SIGNAL(applySignal()), this, SLOT(dlgApply_clicked()));

    SimulationArea ctemArea = *SimManager->getSimulationArea();
    StemArea stemArea = *SimManager->getStemArea();
    CbedPosition cbedPos = *SimManager->getCBedPosition();

    CtemFrame = new CtemAreaFrame(this, ctemArea);
    StemFrame = new StemAreaFrame(this, stemArea);
    CbedFrame = new CbedAreaFrame(this, cbedPos);

    ui->vCtemLayout->insertWidget(0, CtemFrame);
    ui->vStemLayout->insertWidget(0, StemFrame);
    ui->vCbedLayout->insertWidget(0, CbedFrame);

    connect(CtemFrame, SIGNAL(areaChanged()), this, SLOT(areasChanged()));
    connect(StemFrame, SIGNAL(areaChanged()), this, SLOT(areasChanged()));
    connect(CbedFrame, SIGNAL(areaChanged()), this, SLOT(areasChanged()));

    connect(CtemFrame, SIGNAL(applyChanges()), this, SLOT(apply_pressed()));
    connect(StemFrame, SIGNAL(applyChanges()), this, SLOT(apply_pressed()));
    connect(CbedFrame, SIGNAL(applyChanges()), this, SLOT(apply_pressed()));

    connect(ui->tabAreaWidget, SIGNAL(currentChanged(int)), this, SLOT(areasChanged()));

    // set current tab to view
    auto mode = SimManager->getMode();
    if (mode == SimulationMode::STEM)
        ui->tabAreaWidget->setCurrentIndex(1);
    else if (mode == SimulationMode::CBED)
        ui->tabAreaWidget->setCurrentIndex(2);
    else
        ui->tabAreaWidget->setCurrentIndex(0);

    // set resolution combo box
    // this has to be called here as changingit will call its slot when other values havent been initialised
    int ind = ui->cmbResolution->findText( QString::number(SimManager->getResolution()) );
    ui->cmbResolution->setCurrentIndex(ind);

    areasChanged();
}

AreaLayoutFrame::~AreaLayoutFrame()
{
    delete ui;
}

void AreaLayoutFrame::areasChanged() {
    // depending one mode selected, calculate the simulation area (with padding etc)
    // should be part of the simmanager?
    auto mode = ui->tabAreaWidget->currentIndex();

    // update the mode on the main window if it needs doing :)
    // should probably be it's own slot
    emit modeChanged(mode);

    float realScale = 0.0f;

    if (mode == 0) { // CTEM
        auto sa = CtemFrame->getSimArea(); // this is just the user set area, no padding etc
        auto xlims = sa.getLimitsX();
        auto range = xlims[1] - xlims[0];
        realScale = SimManager->calculatePaddedRealScale(range, SimManager->getResolution(), true);
    }
    else if (mode == 1) { // STEM
        auto stema = StemFrame->getStemArea();
        auto sa = stema.getSimArea();
        auto xlims = sa.getLimitsX();
        auto range = xlims[1] - xlims[0]; // x lims should be the same as y
        realScale = SimManager->calculatePaddedRealScale(range, SimManager->getResolution() );

        ui->lblStemScaleX->setText(Utils::numToQString(stema.getScaleX(), lbl_precision) + " Å");
        ui->lblStemScaleY->setText(Utils::numToQString(stema.getScaleY(), lbl_precision) + " Å");
    }
    else if (mode == 2) { // CBED
        auto pos = CbedFrame->getCbedPos();
        auto sa = pos.getSimArea();
        auto xlims = sa.getLimitsX();
        auto range = xlims[1] - xlims[0]; // x lims should be the same as y
        realScale = SimManager->calculatePaddedRealScale(range, SimManager->getResolution() );
    }

    if (mode == 1) {
        ui->lblStemXHeader->setVisible(true);
        ui->lblStemYHeader->setVisible(true);
        ui->lblStemScaleX->setVisible(true);
        ui->lblStemScaleY->setVisible(true);
    } else {
        ui->lblStemXHeader->setVisible(false);
        ui->lblStemYHeader->setVisible(false);
        ui->lblStemScaleX->setVisible(false);
        ui->lblStemScaleY->setVisible(false);
    }

    double freqScale = 1.0 / (realScale  * SimManager->getResolution());
    double freqMax = 0.5 * freqScale * SimManager->getResolution() * SimManager->getInverseLimitFactor();
    double angleScale = freqScale * SimManager->getWavelength() * 1000.0;
    double angleMax = freqMax * SimManager->getWavelength() * 1000.0;

    ui->lblRealScale->setText(Utils::numToQString(realScale, lbl_precision) + " Å");
    ui->lblFreqScale->setText(Utils::numToQString(freqScale, lbl_precision) + " Å<sup>-1</sup>");
    ui->lblFreqMax->setText(Utils::numToQString(freqMax, lbl_precision) + " Å<sup>-1</sup>");
    ui->lblAngleScale->setText(Utils::numToQString(angleScale, lbl_precision) + " mrad");
    ui->lblAngleMax->setText(Utils::numToQString(angleMax, lbl_precision) + " mrad");
}

void AreaLayoutFrame::on_cmbResolution_currentIndexChanged(const QString &arg1) {
    int res = arg1.toInt();
    SimManager->setResolution(res);
    areasChanged();
    // this will set the resolution again, but is easiest way of updating the other combo box
    emit resolutionChanged(arg1);
}

void AreaLayoutFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    parentWidget()->close();
}

void AreaLayoutFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    dlgApply_clicked();
    parentWidget()->close();
}

void AreaLayoutFrame::dlgApply_clicked()
{
    apply_pressed();
}

void AreaLayoutFrame::apply_pressed() {

    // get which mode we are in
    auto mode = ui->tabAreaWidget->currentIndex();

    if (mode == 0) { // CTEM
        auto sa = CtemFrame->getSimArea();
        *SimManager->getSimulationArea() = sa;
        // update the frame so it has the correct reset point
        CtemFrame->updateCurrentArea(sa);
    }
    else if (mode == 1) { // STEM
        auto stema = StemFrame->getStemArea();
        *SimManager->getStemArea() = stema;
        StemFrame->updateCurrentArea(stema);
        emit updateMainStem();
    }
    else if (mode == 2) { // CBED
        auto pos = CbedFrame->getCbedPos();
        *SimManager->getCBedPosition() = pos;
        CbedFrame->updateCurrentArea(pos);
        emit updateMainCbed();
    }
}

bool AreaLayoutFrame::getErrorStringCtem() {
    // check the range is positive non zero

    // check the scale is positive non zero and not inf

    return false;
}

bool AreaLayoutFrame::getErrorStringCbed() {
    // check the padding is positive

    // check the scale is positive non zero and not inf
}

bool AreaLayoutFrame::getErrorStringStem() {
    // check the padding is positive

    // check the ranges are positive non zero

    // get pixels are positive non zero

    // check the scale is positive non zero and not inf
}
