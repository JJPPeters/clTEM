#include <utils/stringutils.h>
#include "arealayoutframe.h"
#include "ui_arealayoutframe.h"

AreaLayoutFrame::AreaLayoutFrame(QWidget *parent, std::shared_ptr<SimulationManager> simMan) :
    QWidget(parent), SimManager(simMan),
    ui(new Ui::AreaLayoutFrame)
{
    ui->setupUi(this);

    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtSliceThickness->setValidator(pValidator);
    ui->edtSliceOffset->setValidator(pValidator);

    ui->edtSliceThickness->setUnits("Å");
    ui->edtSliceOffset->setUnits("Å");

    connect(ui->edtSliceThickness, SIGNAL(textChanged(QString)), this, SLOT(checkEditZero(QString)));
    connect(ui->edtSliceOffset, SIGNAL(textChanged(QString)), this, SLOT(checkEditZero(QString)));

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

        ui->lblStemScaleX->setText(Utils_Qt::numToQString(stema.getScaleX()) + " Å");
        ui->lblStemScaleY->setText(Utils_Qt::numToQString(stema.getScaleY()) + " Å");
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

    ui->lblRealScale->setText(Utils_Qt::numToQString(realScale) + " Å");
    ui->lblFreqScale->setText(Utils_Qt::numToQString(freqScale) + " Å<sup>-1</sup>");
    ui->lblFreqMax->setText(Utils_Qt::numToQString(freqMax) + " Å<sup>-1</sup>");
    ui->lblAngleScale->setText(Utils_Qt::numToQString(angleScale) + " mrad");
    ui->lblAngleMax->setText(Utils_Qt::numToQString(angleMax) + " mrad");

    float dz = SimManager->getSliceThickness();
    float oz = SimManager->getSliceOffset();

    ui->edtSliceThickness->setText(Utils_Qt::numToQString(dz));
    ui->edtSliceOffset->setText(Utils_Qt::numToQString(oz));
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
    bool valid = apply_pressed();
    if (valid)
        parentWidget()->close();
}

void AreaLayoutFrame::dlgApply_clicked()
{
    apply_pressed();
}

bool AreaLayoutFrame::apply_pressed() {

    float dz = ui->edtSliceThickness->text().toFloat();
    float oz = ui->edtSliceOffset->text().toFloat();

    bool valid = true;
    std::vector<std::string> errors;

    if (dz <= 0)
    {
        errors.push_back("Slice thickness must be greater than 0");
        valid = false;
    }
    if (oz < 0)
    {
        errors.push_back("Slice offset must be positive");
        valid = false;
    }

    if (!valid)
    {
        QMessageBox msgBox;
        msgBox.setText("Error:");
        std::string msg = "";
        for (int i = 0; i < errors.size(); ++i)
        {
            msg += errors[i];
            if (i < errors.size() - 1)
                msg += "\n";
        }

        msgBox.setInformativeText(QString::fromStdString(msg));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return valid;
    }

    SimManager->setSliceThickness(dz);
    SimManager->setSliceOffset(oz);

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

    return valid;
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

void AreaLayoutFrame::checkEditZero(QString txt) {
    if (ui->edtSliceThickness->text().toFloat() > 0)
        ui->edtSliceThickness->setStyleSheet("");
    else
        ui->edtSliceThickness->setStyleSheet("color: #FF8C00");

    if (ui->edtSliceOffset->text().toFloat() >= 0)
        ui->edtSliceOffset->setStyleSheet("");
    else
        ui->edtSliceOffset->setStyleSheet("color: #FF8C00");
}
