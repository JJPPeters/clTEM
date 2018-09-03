#include <utils/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
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

    ui->chkKeep->setChecked(simMan->getMaintainAreas());

    connect(ui->edtSliceThickness, &QLineEdit::textChanged, this, &AreaLayoutFrame::checkEditZero);
    connect(ui->edtSliceOffset, &QLineEdit::textChanged, this, &AreaLayoutFrame::checkEditZero);

    auto parent_dlg = dynamic_cast<SimAreaDialog*>(parentWidget());
    connect(parent_dlg, &SimAreaDialog::okSignal, this, &AreaLayoutFrame::dlgOk_clicked);
    connect(parent_dlg, &SimAreaDialog::cancelSignal, this, &AreaLayoutFrame::dlgCancel_clicked);
    connect(parent_dlg, &SimAreaDialog::applySignal, this, &AreaLayoutFrame::dlgApply_clicked);

    SimulationArea ctemArea = *SimManager->getSimulationArea();
    StemArea stemArea = *SimManager->getStemArea();
    CbedPosition cbedPos = *SimManager->getCBedPosition();

    CtemFrame = new CtemAreaFrame(this, ctemArea);
    StemFrame = new StemAreaFrame(this, stemArea);
    CbedFrame = new CbedAreaFrame(this, cbedPos);

    ui->vCtemLayout->insertWidget(0, CtemFrame);
    ui->vStemLayout->insertWidget(0, StemFrame);
    ui->vCbedLayout->insertWidget(0, CbedFrame);

    connect(CtemFrame, &CtemAreaFrame::areaChanged, this, &AreaLayoutFrame::areasChanged);
    connect(StemFrame, &StemAreaFrame::areaChanged, this, &AreaLayoutFrame::areasChanged);
    connect(CbedFrame, &CbedAreaFrame::areaChanged, this, &AreaLayoutFrame::areasChanged);

    connect(CtemFrame, &CtemAreaFrame::applyChanges, this, &AreaLayoutFrame::apply_pressed);
    connect(StemFrame, &StemAreaFrame::applyChanges, this, &AreaLayoutFrame::apply_pressed);
    connect(CbedFrame, &CbedAreaFrame::applyChanges, this, &AreaLayoutFrame::apply_pressed);

    connect(ui->tabAreaWidget, &QTabWidget::currentChanged, this, &AreaLayoutFrame::areasChanged);

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

    setStructLimits();

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

    connect(ui->edtSliceThickness, &QLineEdit::textChanged, this, &AreaLayoutFrame::slicesChanged);
    connect(ui->edtSliceOffset, &QLineEdit::textChanged, this, &AreaLayoutFrame::slicesChanged);

    ui->edtSliceThickness->setText(Utils_Qt::numToQString(dz));
    ui->edtSliceOffset->setText(Utils_Qt::numToQString(oz));

    //ui->pltStructure->setOpenGl(true);
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

    SimManager->setMaintainAreas(ui->chkKeep->isChecked());

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

    emit areaChanged();

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

void AreaLayoutFrame::setStructLimits() {
    if (!SimManager->getStructure())
        return;

    auto lims_x = SimManager->getStructure()->getLimitsX();
    auto lims_y = SimManager->getStructure()->getLimitsY();

    ui->lblStructStartX->setText(Utils_Qt::numToQString(lims_x[0]) + " Å");
    ui->lblStructFinishX->setText(Utils_Qt::numToQString(lims_x[1]) + " Å");
    ui->lblStructStartY->setText(Utils_Qt::numToQString(lims_y[0]) + " Å");
    ui->lblStructFinishY->setText(Utils_Qt::numToQString(lims_y[1]) + " Å");
}

void AreaLayoutFrame::slicesChanged() {
    if (!SimManager->getStructure())
        return;

    float dz = ui->edtSliceThickness->text().toFloat();

    auto z_lims = SimManager->getPaddedStructLimitsZ();
    float z_range = z_lims[1] - z_lims[0];

    unsigned int n_slices = (unsigned int) std::ceil(z_range / dz);
    n_slices += (n_slices == 0);

    ui->lblSlices->setText(Utils_Qt::numToQString(n_slices));

}

void AreaLayoutFrame::plotStructure() {

    // test if we have a structure to plot...
    if (!SimManager->getStructure())
        return;

    auto atms = SimManager->getStructure()->getAtoms();

    std::vector<Vector3f> pos(atms.size());
    std::vector<Vector3f> col(atms.size());

    for (int i = 0; i < atms.size(); ++i) {
        pos[i] = Vector3f(atms[i].x, atms[i].y, atms[i].z);

        auto qc = GuiUtils::ElementNumberToQColour(atms[i].A);
        col[i] = Vector3f(qc.red(), qc.green(), qc.blue());
    }

    ui->pltStructure->PlotAtoms(pos, col, View::Direction::Top);

//    // TODO: might want to do on construction (in case we need to update)
//    ui->pltStructure->setInteraction(QCP::iRangeDrag, true);
//    ui->pltStructure->setInteraction(QCP::iRangeZoom, true);
//    ui->pltStructure->axisRect()->setupFullAxesBox();
//
//    // test if we have a structure to plot...
//    if (!SimManager->getStructure())
//        return;
//
//    std::vector<QCPCurve *> curves;
//
//    auto atm_t = SimManager->getStructure()->getAtomsTypes();
//    for (int i = 0; i < atm_t.size(); ++i) {
//
//        QCPCurve *mycurve = new QCPCurve(ui->pltStructure->xAxis, ui->pltStructure->yAxis);
//
//        auto ss = QCPScatterStyle(QCPScatterStyle::ScatterShape::ssCircle, QColor(127,127,127), GuiUtils::ElementNumberToQColour(atm_t[i]), 7);
//
//        mycurve->setLineStyle(QCPCurve::lsNone);
//        mycurve->setScatterStyle(ss);
//        curves.push_back(mycurve);
//    }
//
//    // get our atoms
//    auto atms = SimManager->getStructure()->getAtoms();
//    std::sort(atms.begin(), atms.end(), AtomSite_z_less_Sort());


//    // sort through them all
//    std::vector<AtomSite> shown;
//    float diff_lim = 0.01f;
//    for (const auto &a : atms) {
//        bool add = true;
//        float xa = a.x;
//        float ya = a.y;
//        float za = a.z;
//
//        std::vector<int> to_delete;
//        for (int i = 0; i < shown.size(); ++i) {
//            float xs = shown[i].x;
//            float ys = shown[i].y;
//            float zs = shown[i].z;
//
//            // if atoms are closer than our limits in x/y
//            if (std::abs(xs-xa) < diff_lim && std::abs(ys-ya) < diff_lim) {
//                if (za > zs) {
//                    // mark shown atoms for deletion
//                    to_delete.push_back(i);
//                } else {
//                    add = false;
//                }
//            }
//        }
//
//        // delete atoms from our shown list
//        // we know because of how we looped through, that our delete is list in ascending order
//        for (int i = 0; i < to_delete.size(); ++i) {
//            shown.erase(shown.begin() + to_delete[to_delete.size() - 1 - i]);
//        }
//
//        if (add)
//            shown.emplace_back(a);
//    }

//    for (const auto &a : atms) {
//        int pos = static_cast<int>(std::find(atm_t.begin(), atm_t.end(), a.A) - atm_t.begin());
//        if (pos < atm_t.size())
//            curves[pos]->addData(a.x, a.y);
//    }
//
//    ui->pltStructure->rescaleAxes();
//
//    // TODO: need to set this to keep all the data in the view
//    // I think it depends on the window size, as well as the data range...
//    // for now, this gives the right aspect ratio
//    ui->pltStructure->xAxis->setScaleRatio(ui->pltStructure->yAxis, 1);
//
//    ui->pltStructure->replot(QCustomPlot::rpQueuedReplot);
}

void AreaLayoutFrame::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    plotStructure();
}
