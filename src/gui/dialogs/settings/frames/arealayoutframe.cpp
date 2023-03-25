#include <utils/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include "arealayoutframe.h"
#include "ui_arealayoutframe.h"

AreaLayoutFrame::AreaLayoutFrame(QWidget *parent, std::shared_ptr<SimulationManager> simMan) :
    QWidget(parent), ui(new Ui::AreaLayoutFrame), SimManager(simMan)
{
    ui->setupUi(this);

    // this is just from theQt website, Not really sure what it does,
    // but it makes OpenGL work on my linux laptop (intel 4th gen)
    try {
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setVersion(4, 0); // sets opengl version

        QSettings settings;
        int msaa = settings.value("MSAA", 1).toInt();

        pltStructure = std::make_shared<PGL::PlotWidget>(this, msaa);
        pltStructure->setFormat(format);
        ui->vPlotLayout->addWidget(pltStructure.get(), 1);
        pltStructure->setMinimumWidth(400);

        connect(pltStructure.get(), &PGL::PlotWidget::resetView, this, &AreaLayoutFrame::viewDirectionChanged);
        connect(pltStructure.get(), &PGL::PlotWidget::initError, this, &AreaLayoutFrame::processOpenGLError);
    } catch (const std::exception& e) {
        CLOG(WARNING, "gui") << "Failed to make OpenGL view: " << e.what();
        QMessageBox msgBox(this);
        msgBox.setText("Error:");
        msgBox.setInformativeText(e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setMinimumSize(160, 125);
        msgBox.exec();
    }

    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtSliceThickness->setValidator(pValidator);
    ui->edtSliceOffset->setValidator(pValidator);

    ui->edtSliceThickness->setUnits("Å");
    ui->edtSliceOffset->setUnits("Å");

    ui->chkKeep->setChecked(simMan->maintainAreas());

    connect(ui->edtSliceThickness, &QLineEdit::textChanged, this, &AreaLayoutFrame::checkEditZero);
    connect(ui->edtSliceOffset, &QLineEdit::textChanged, this, &AreaLayoutFrame::checkEditZero);
    connect(ui->edtSliceOutput, &QLineEdit::textChanged, this, &AreaLayoutFrame::checkEditZero);

    auto parent_dlg = dynamic_cast<SimAreaDialog*>(parentWidget());
    connect(parent_dlg, &SimAreaDialog::okSignal, this, &AreaLayoutFrame::dlgOk_clicked);
    connect(parent_dlg, &SimAreaDialog::cancelSignal, this, &AreaLayoutFrame::dlgCancel_clicked);
    connect(parent_dlg, &SimAreaDialog::applySignal, this, &AreaLayoutFrame::dlgApply_clicked);

    SimulationArea ctemArea = *SimManager->simulationArea();
    StemArea stemArea = *SimManager->stemArea();
    CbedPosition cbedPos = *SimManager->cbedPosition();

    CtemFrame = new CtemAreaFrame(this, ctemArea, SimManager->simulationCell()->crystalStructure());
    StemFrame = new StemAreaFrame(this, stemArea, SimManager->simulationCell()->crystalStructure());
    CbedFrame = new CbedAreaFrame(this, cbedPos, SimManager->simulationCell()->crystalStructure());

    ui->vCtemLayout->insertWidget(0, CtemFrame);
    ui->vStemLayout->insertWidget(0, StemFrame);
    ui->vCbedLayout->insertWidget(0, CbedFrame);

    // set current tab to view
    auto mode = SimManager->mode();
    if (mode == SimulationMode::STEM)
        ui->tabAreaWidget->setCurrentIndex(1);
    else if (mode == SimulationMode::CBED)
        ui->tabAreaWidget->setCurrentIndex(2);
    else
        ui->tabAreaWidget->setCurrentIndex(0);

    // These must be called so they don't try to update the OGL plot before it is shown (which is baaaad)
    connect(ui->tabAreaWidget, &QTabWidget::currentChanged, this, &AreaLayoutFrame::areasChanged);
    connect(ui->tabAreaWidget, &QTabWidget::currentChanged, this, &AreaLayoutFrame::updatePlotRects);

    connect(CtemFrame, &CtemAreaFrame::areaChanged, this, &AreaLayoutFrame::areasChanged);
    connect(StemFrame, &StemAreaFrame::areaChanged, this, &AreaLayoutFrame::areasChanged);
    connect(CbedFrame, &CbedAreaFrame::areaChanged, this, &AreaLayoutFrame::areasChanged);

    connect(ui->chkShowRect, &QCheckBox::stateChanged, this, &AreaLayoutFrame::showRectChanged);

    // set resolution combo box
    // this has to be called here as changingit will call its slot when other values havent been initialised
    int ind = ui->cmbResolution->findText( QString::number(SimManager->resolution()) );
    ui->cmbResolution->setCurrentIndex(ind);

    setStructLimits();

    areasChanged();

    updateSlices();
}



AreaLayoutFrame::~AreaLayoutFrame()
{
//    delete pltStructure;
    pltStructure.reset();
    delete ui;
}

void AreaLayoutFrame::areasChanged() {
    // depending one mode selected, calculate the simulation area (with padding etc)
    // should be part of the simmanager?
    auto mode = ui->tabAreaWidget->currentIndex();

    // update the mode on the main window if it needs doing :)
    // should probably be it's own slot
    emit modeChanged(mode);

    if (mode == 1) { // STEM
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

    // Note that I cant call methods of manager here because the values may not have been set yet.
    // I should maybe move these to their own functions that everything accesses though?

    double realScale = 0.0f;

    // assuming padding is the same in x and y
    auto pd = SimManager->simulationCell()->paddingX();
    auto pd_range = std::abs(pd[1]) + std::abs(pd[0]);

    if (mode == 0) { // CTEM
        auto sa = CtemFrame->getSimArea(); // this is just the user set area, no padding etc
        auto xlims = sa.getCorrectedLimitsX();
        auto range = xlims[1] - xlims[0];
        realScale = (range + pd_range) / SimManager->resolution();
    }
    else if (mode == 1) { // STEM
        auto stema = StemFrame->stemArea();
        auto xlims = stema.getCorrectedLimitsX();
        auto range = xlims[1] - xlims[0]; // x lims should be the same as y
        realScale = (range + pd_range) / SimManager->resolution();

        ui->lblStemScaleX->setText(Utils_Qt::numToQString(stema.getScaleX()) + " Å");
        ui->lblStemScaleY->setText(Utils_Qt::numToQString(stema.getScaleY()) + " Å");
    }
    else if (mode == 2) { // CBED
        auto pos = CbedFrame->getCbedPos();
        auto sa = pos.getSimArea();
        auto xlims = sa.getCorrectedLimitsX();
        auto range = xlims[1] - xlims[0]; // x lims should be the same as y
        realScale = (range + pd_range) / SimManager->resolution();
    }

    double freqScale = 1.0 / (realScale  * SimManager->resolution());
    double freqMax = 0.5 * freqScale * SimManager->resolution() * SimManager->inverseLimitFactor();
    double angleScale = freqScale * SimManager->microscopeParams()->Wavelength() * 1000.0;
    double angleMax = freqMax * SimManager->microscopeParams()->Wavelength() * 1000.0;

    ui->lblRealScale->setText(Utils_Qt::numToQString(realScale) + " Å");
    ui->lblFreqScale->setText(Utils_Qt::numToQString(freqScale) + " Å<sup>-1</sup>");
    ui->lblFreqMax->setText(Utils_Qt::numToQString(freqMax) + " Å<sup>-1</sup>");
    ui->lblAngleScale->setText(Utils_Qt::numToQString(angleScale) + " mrad");
    ui->lblAngleMax->setText(Utils_Qt::numToQString(angleMax) + " mrad");

    slicesChanged();
}

void AreaLayoutFrame::updateSlices() {
    double dz = SimManager->simulationCell()->sliceThickness();
    double oz = SimManager->simulationCell()->sliceOffset();
    unsigned int so = SimManager->storedIntermediateSliceStep();
    bool iso = SimManager->intermediateSlicesEnabled();

    connect(ui->edtSliceThickness, &QLineEdit::textChanged, this, &AreaLayoutFrame::slicesChanged);
    connect(ui->edtSliceOffset, &QLineEdit::textChanged, this, &AreaLayoutFrame::slicesChanged);

    ui->edtSliceThickness->setText(Utils_Qt::numToQString(dz));
    ui->edtSliceOffset->setText(Utils_Qt::numToQString(oz));

    ui->edtSliceOutput->setText(Utils_Qt::numToQString(so));
    ui->chkSliceOutput->setChecked(iso);
}

void AreaLayoutFrame::on_cmbResolution_currentIndexChanged(const QString &arg1) {
    int res = arg1.toInt();
    SimManager->setResolution(static_cast<unsigned int>(res));
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

    double dz = ui->edtSliceThickness->text().toDouble();
    double oz = ui->edtSliceOffset->text().toDouble();

    unsigned int so = ui->edtSliceOutput->text().toUInt();
    bool iso = ui->chkSliceOutput->isChecked();

    bool valid = true;
    std::vector<std::string> errors;

    if (dz <= 0) {
        errors.emplace_back("Slice thickness must be greater than 0");
        valid = false;
    }
    if (oz < 0) {
        errors.emplace_back("Slice offset must be positive");
        valid = false;
    }

    if (so < 0) {
        errors.emplace_back("Intermediate slice output must be positive");
        valid = false;
    }

    if (!valid)
    {
        QMessageBox msgBox;
        msgBox.setText("Error:");
        std::string msg = "";
        for (size_t i = 0; i < errors.size(); ++i)
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

    SimManager->simulationCell()->setSliceThickness(dz);
    SimManager->simulationCell()->setSliceOffset(oz);
    SimManager->setIntermediateSlices(so);
    SimManager->setIntermediateSlicesEnabled(iso);

    SimManager->setMaintainAreas(ui->chkKeep->isChecked());

    // get which mode we are in
    auto mode = ui->tabAreaWidget->currentIndex();

    if (mode == 0) { // CTEM
        auto sa = CtemFrame->getSimArea();
        *SimManager->simulationArea() = sa;
        // update the frame so it has the correct reset point
        CtemFrame->updateCurrentArea(sa);
    }
    else if (mode == 1) { // STEM
        auto stema = StemFrame->stemArea();
        *SimManager->stemArea() = stema;
        StemFrame->updateCurrentArea(stema);
        emit updateMainStem();
    }
    else if (mode == 2) { // CBED
        auto pos = CbedFrame->getCbedPos();
        *SimManager->cbedPosition() = pos;
        CbedFrame->updateCurrentArea(pos);
        emit updateMainCbed();
    }

    emit areaChanged();

    updatePlotRects();

    return valid;
}

//bool AreaLayoutFrame::getErrorStringCtem() {
//    // check the range is positive non zero
//
//    // check the scale is positive non zero and not inf
//
//    return false;
//}
//
//bool AreaLayoutFrame::getErrorStringCbed() {
//    // check the padding is positive
//
//    // check the scale is positive non zero and not inf
//    return false;
//}
//
//bool AreaLayoutFrame::getErrorStringStem() {
//    // check the padding is positive
//
//    // check the ranges are positive non zero
//
//    // get pixels are positive non zero
//
//    // check the scale is positive non zero and not inf
//    return false;
//}

void AreaLayoutFrame::checkEditZero(QString txt) {
    if (ui->edtSliceThickness->text().toDouble() > 0)
        ui->edtSliceThickness->setStyleSheet("");
    else
        ui->edtSliceThickness->setStyleSheet("color: #FF8C00");

    if (ui->edtSliceOutput->text().toInt() > 0)
        ui->edtSliceOutput->setStyleSheet("");
    else
        ui->edtSliceOutput->setStyleSheet("color: #FF8C00");

    if (ui->edtSliceOffset->text().toDouble() >= 0)
        ui->edtSliceOffset->setStyleSheet("");
    else
        ui->edtSliceOffset->setStyleSheet("color: #FF8C00");
}

void AreaLayoutFrame::setStructLimits() {
    if (!SimManager->simulationCell()->crystalStructure())
        return;

    auto lims_x = SimManager->simulationCell()->crystalStructure()->limitsX();
    auto lims_y = SimManager->simulationCell()->crystalStructure()->limitsY();

    ui->lblStructStartX->setText(Utils_Qt::numToQString(lims_x[0]) + " Å");
    ui->lblStructFinishX->setText(Utils_Qt::numToQString(lims_x[1]) + " Å");
    ui->lblStructStartY->setText(Utils_Qt::numToQString(lims_y[0]) + " Å");
    ui->lblStructFinishY->setText(Utils_Qt::numToQString(lims_y[1]) + " Å");
}

void AreaLayoutFrame::slicesChanged() {
    if (!SimManager->simulationCell()->crystalStructure())
        return;

    double dz = ui->edtSliceThickness->text().toDouble();

    auto z_lims = SimManager->paddedSimLimitsZ();
    double z_range = z_lims[1] - z_lims[0];

    auto n_slices = (unsigned int) std::ceil(z_range / dz);
    n_slices += (n_slices == 0);

    ui->lblSlices->setText(Utils_Qt::numToQString(n_slices));

}

void AreaLayoutFrame::plotStructure() {

    // test if we have a structure to plot...
    if (!SimManager->simulationCell()->crystalStructure() || !pltStructure)
        return;

    // get ranges (needed to define out 'cube'
    auto xr = SimManager->simulationCell()->crystalStructure()->limitsX();
    auto yr = SimManager->simulationCell()->crystalStructure()->limitsY();
    auto zr = SimManager->simulationCell()->crystalStructure()->limitsZ();

    auto atms = SimManager->simulationCell()->crystalStructure()->atoms();

    std::vector<Eigen::Vector3f> pos(atms.size());
    std::vector<Eigen::Vector3f> col(atms.size());

    for (size_t i = 0; i < atms.size(); ++i) {
        pos[i] = Eigen::Vector3f(atms[i].x, atms[i].y, atms[i].z);

        auto qc = GuiUtils::ElementNumberToQColour(atms[i].A);
        col[i] = Eigen::Vector3f(qc.red(), qc.green(), qc.blue()) / 255.0f;
    }

    // here is where the data is actually plotted
    _plot_scatter = pltStructure->scatter(pos, col);

    pltStructure->SetViewDirection(View::Direction::Top);
    pltStructure->FitView(1.1);
    pltStructure->repaint();
}

void AreaLayoutFrame::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    // test if we have a structure to plot...
    // This is mostly for the fitView method (the others protect themselves)
    if (!SimManager->simulationCell()->crystalStructure())
        return;

    plotStructure();
    updatePlotRects();
}

void AreaLayoutFrame::on_cmbViewDirection_activated(const QString &arg1) {
    viewDirectionChanged();
}

void AreaLayoutFrame::viewDirectionChanged() {
    if (!pltStructure)
        return;

    QString view_text = ui->cmbViewDirection->currentText();

    // set the view direcion of the plot
    if (view_text == "Top")
        pltStructure->SetViewDirection(View::Direction::Top);
    else if (view_text == "Front")
        pltStructure->SetViewDirection(View::Direction::Front);
    else if (view_text == "Right")
        pltStructure->SetViewDirection(View::Direction::Right);
    else if (view_text == "Bottom")
        pltStructure->SetViewDirection(View::Direction::Bottom);
    else if (view_text == "Back")
        pltStructure->SetViewDirection(View::Direction::Back);
    else if (view_text == "Left")
        pltStructure->SetViewDirection(View::Direction::Left);

    pltStructure->FitView(1.1);

    pltStructure->repaint();
}

void AreaLayoutFrame::showRectChanged(int arg1) {
    if (!pltStructure)
        return;

    for(auto &rect: _plot_rects) {
        auto r = rect.lock();
        if (r)
            r->setVisible(arg1 != 0);
    }

    pltStructure->repaint();
}

void AreaLayoutFrame::updatePlotRects() {
    // Add in the rectangles showing the simulation areas and slices

    // test if we have a structure to plot...
    if (!SimManager->simulationCell()->crystalStructure() || !pltStructure)
        return;

    // clear the old stuff first
    for (auto &rect: _plot_rects)
        pltStructure->removeItem(rect);

    _plot_rects.clear();

    auto test = SimManager->simulationArea();

    auto szr = SimManager->paddedSimLimitsZ();
    auto ixr = SimManager->rawFullLimitsX();
    auto iyr = SimManager->rawFullLimitsY();
    auto sxr = SimManager->paddedFullLimitsX();
    auto syr = SimManager->paddedFullLimitsY();

    // get these now so we know how many we will have
    auto dz = SimManager->simulationCell()->sliceThickness();
    auto nz = SimManager->simulationCell()->sliceCount();

    unsigned int rect_count = 4 * nz + 4;

    _plot_rects.reserve(rect_count);

    // first the sim area + padding
    Eigen::Vector4f col_1 = Eigen::Vector4f(0.0f, 0.5f, 1.0f, 0.1f);
    _plot_rects.emplace_back(pltStructure->rectangle(syr[0], sxr[0], syr[1], sxr[1], szr[0], col_1, PGL::Plane::z));
    _plot_rects.emplace_back(pltStructure->rectangle(syr[0], sxr[0], syr[1], sxr[1], szr[1], col_1, PGL::Plane::z));

    // now the sim area
    Eigen::Vector4f col_2 = Eigen::Vector4f(1.0f, 0.4f, 0.0f, 0.1f);
    _plot_rects.emplace_back(pltStructure->rectangle(iyr[0], ixr[0], iyr[1], ixr[1], szr[0], col_2, PGL::Plane::z));
    _plot_rects.emplace_back(pltStructure->rectangle(iyr[0], ixr[0], iyr[1], ixr[1], szr[1], col_2, PGL::Plane::z));

    // now add the sides of the sim area
    _plot_rects.emplace_back(pltStructure->rectangle(szr[0], iyr[0], szr[1], iyr[1], ixr[0], col_2, PGL::Plane::x));
    _plot_rects.emplace_back(pltStructure->rectangle(szr[0], iyr[0], szr[1], iyr[1], ixr[1], col_2, PGL::Plane::x));

    _plot_rects.emplace_back(pltStructure->rectangle(szr[0], ixr[0], szr[1], ixr[1], iyr[0], col_2, PGL::Plane::y));
    _plot_rects.emplace_back(pltStructure->rectangle(szr[0], ixr[0], szr[1], ixr[1], iyr[1], col_2, PGL::Plane::y));

    // add the slices

    std::vector<Eigen::Vector4f> cols_slice = {Eigen::Vector4f(1.0f, 1.0f, 0.0f, 0.1f), Eigen::Vector4f(0.3f, 0.7f, 0.4f, 0.1f)};

    auto current_z = szr[0];
    for (size_t i = 0; i < nz; ++i) {
        auto current_col = cols_slice[i % 2];
        _plot_rects.emplace_back(pltStructure->rectangle(current_z, syr[0], current_z + dz, syr[1], sxr[0], current_col, PGL::Plane::x));
        _plot_rects.emplace_back(pltStructure->rectangle(current_z, syr[0], current_z + dz, syr[1], sxr[1], current_col, PGL::Plane::x));

        _plot_rects.emplace_back(pltStructure->rectangle(current_z, sxr[0], current_z + dz, sxr[1], syr[0], current_col, PGL::Plane::y));
        _plot_rects.emplace_back(pltStructure->rectangle(current_z, sxr[0], current_z + dz, sxr[1], syr[1], current_col, PGL::Plane::y));

        current_z += dz;
    }

    pltStructure->FitView(1.1);
    pltStructure->repaint();

}

void AreaLayoutFrame::on_btnApplyUpdate_clicked() {
    apply_pressed();
}

void AreaLayoutFrame::processOpenGLError(std::string message) {
    CLOG(WARNING, "gui") << "OpenGL initialisation: " << message;
    QMessageBox msgBox(this);
    msgBox.setText("Error:");
    msgBox.setInformativeText(QString::fromStdString(message));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setMinimumSize(160, 125);
    msgBox.exec();
}
