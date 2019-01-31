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
//        format.setDepthBufferSize(24);
//        format.setStencilBufferSize(8);
        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setProfile(QSurfaceFormat::CoreProfile);
//        format.setSamples(32); // sets MSAA samples
        format.setVersion(4, 0); // sets opengl version

        pltStructure = new OGLViewWidget(this);
        pltStructure->setFormat(format);
        ui->vPlotLayout->addWidget(pltStructure, 0);
        pltStructure->setMinimumWidth(400);
        connect(pltStructure, &OGLViewWidget::resetView, this, &AreaLayoutFrame::viewDirectionChanged);
        connect(pltStructure, &OGLViewWidget::initError, this, &AreaLayoutFrame::processOpenGLError);
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

    CtemFrame = new CtemAreaFrame(this, ctemArea, SimManager->getStructure());
    StemFrame = new StemAreaFrame(this, stemArea, SimManager->getStructure());
    CbedFrame = new CbedAreaFrame(this, cbedPos, SimManager->getStructure());

    ui->vCtemLayout->insertWidget(0, CtemFrame);
    ui->vStemLayout->insertWidget(0, StemFrame);
    ui->vCbedLayout->insertWidget(0, CbedFrame);

    // set current tab to view
    auto mode = SimManager->getMode();
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
    int ind = ui->cmbResolution->findText( QString::number(SimManager->getResolution()) );
    ui->cmbResolution->setCurrentIndex(ind);

    setStructLimits();

    areasChanged();
}



AreaLayoutFrame::~AreaLayoutFrame()
{
    delete pltStructure;
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

    auto pd = SimManager->getPaddingX();
    auto pd_range = std::abs(pd[1]) + std::abs(pd[0]);

    if (mode == 0) { // CTEM
        auto sa = CtemFrame->getSimArea(); // this is just the user set area, no padding etc
        auto xlims = sa.getCorrectedLimitsX();
        auto range = xlims[1] - xlims[0];
        realScale = (range + pd_range) / SimManager->getResolution();
    }
    else if (mode == 1) { // STEM
        auto stema = StemFrame->getStemArea();
        auto xlims = stema.getCorrectedLimitsX();
        auto range = xlims[1] - xlims[0]; // x lims should be the same as y
        realScale = (range + pd_range) / SimManager->getResolution();

        ui->lblStemScaleX->setText(Utils_Qt::numToQString(stema.getScaleX()) + " Å");
        ui->lblStemScaleY->setText(Utils_Qt::numToQString(stema.getScaleY()) + " Å");
    }
    else if (mode == 2) { // CBED
        auto pos = CbedFrame->getCbedPos();
        auto sa = pos.getSimArea();
        auto xlims = sa.getCorrectedLimitsX();
        auto range = xlims[1] - xlims[0]; // x lims should be the same as y
        realScale = (range + pd_range) / SimManager->getResolution();
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

    float dz = ui->edtSliceThickness->text().toFloat();
    float oz = ui->edtSliceOffset->text().toFloat();

    bool valid = true;
    std::vector<std::string> errors;

    if (dz <= 0)
    {
        errors.emplace_back("Slice thickness must be greater than 0");
        valid = false;
    }
    if (oz < 0)
    {
        errors.emplace_back("Slice offset must be positive");
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

    updatePlotRects();

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

    auto n_slices = (unsigned int) std::ceil(z_range / dz);
    n_slices += (n_slices == 0);

    ui->lblSlices->setText(Utils_Qt::numToQString(n_slices));

}

void AreaLayoutFrame::plotStructure() {

    // test if we have a structure to plot...
    if (!SimManager->getStructure() || !pltStructure)
        return;

    // get ranges (needed to define out 'cube'
    auto xr = SimManager->getStructure()->getLimitsX();
    auto yr = SimManager->getStructure()->getLimitsY();
    auto zr = SimManager->getStructure()->getLimitsZ();

    auto atms = SimManager->getStructure()->getAtoms();

    std::vector<Vector3f> pos(atms.size());
    std::vector<Vector3f> col(atms.size());

    for (int i = 0; i < atms.size(); ++i) {
        pos[i] = Vector3f(atms[i].x, atms[i].y, atms[i].z);

        auto qc = GuiUtils::ElementNumberToQColour(atms[i].A);
        col[i] = Vector3f(qc.red(), qc.green(), qc.blue()) / 255.0f;
    }

    pltStructure->PlotAtoms(pos, col, View::Direction::Top, xr[0], xr[1], yr[0], yr[1], zr[0], zr[1]);
}

void AreaLayoutFrame::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    // test if we have a structure to plot...
    // This is mostly for the fitView method (the others protect themselves)
    if (!SimManager->getStructure())
        return;

    plotStructure();
    updatePlotRects();
    if (!pltStructure)
        pltStructure->fitView();
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

    pltStructure->repaint();
}

void AreaLayoutFrame::showRectChanged(int arg1) {
    if (!pltStructure)
        return;

    pltStructure->setDrawRects(arg1 != 0);
    pltStructure->repaint();
}

void AreaLayoutFrame::updatePlotRects() {
    // Add in the rectangles showing the simulation areas and slices

    // test if we have a structure to plot...
    if (!SimManager->getStructure() || !pltStructure)
        return;

    // clear the old stuff first
    pltStructure->clearRectBuffers();

    auto test = SimManager->getSimulationArea();

    auto szr = SimManager->getPaddedStructLimitsZ();
    auto ixr = SimManager->getRawSimLimitsX();
    auto iyr = SimManager->getRawSimLimitsY();
    auto sxr = SimManager->getPaddedSimLimitsX();
    auto syr = SimManager->getPaddedSimLimitsY();

    pltStructure->SetCube(sxr[0], sxr[1], syr[0], syr[1], szr[0], szr[1]);

    // first the sim area + padding
    Vector4f col_1 = Vector4f(0.0f, 0.5f, 1.0f, 0.1f);

    pltStructure->AddRectBuffer(syr[0], sxr[0], syr[1], sxr[1], szr[0], col_1, OGL::Plane::z);
    pltStructure->AddRectBuffer(syr[0], sxr[0], syr[1], sxr[1], szr[1], col_1, OGL::Plane::z);

    // now the sim area
    Vector4f col_2 = Vector4f(1.0f, 0.4f, 0.0f, 0.1f);

    pltStructure->AddRectBuffer(iyr[0], ixr[0], iyr[1], ixr[1], szr[0], col_2, OGL::Plane::z);
    pltStructure->AddRectBuffer(iyr[0], ixr[0], iyr[1], ixr[1], szr[1], col_2, OGL::Plane::z);

    // add the sides of the sim area
    pltStructure->AddRectBuffer(szr[0], iyr[0], szr[1], iyr[1], ixr[0], col_2, OGL::Plane::x);
    pltStructure->AddRectBuffer(szr[0], iyr[0], szr[1], iyr[1], ixr[1], col_2, OGL::Plane::x);

    pltStructure->AddRectBuffer(szr[0], ixr[0], szr[1], ixr[1], iyr[0], col_2, OGL::Plane::y);
    pltStructure->AddRectBuffer(szr[0], ixr[0], szr[1], ixr[1], iyr[1], col_2, OGL::Plane::y);


    // now add the sides for slices
    auto dz = SimManager->getSliceThickness();
    auto nz = SimManager->getNumberofSlices();
    std::vector<Vector4f> cols_slice = {Vector4f(1.0f, 1.0f, 0.0f, 0.1f), Vector4f(0.3f, 0.7f, 0.4f, 0.1f)};

    auto current_z = szr[0];
    for (int i = 0; i < nz; ++i) {
        auto current_col = cols_slice[i % 2];

        pltStructure->AddRectBuffer(current_z, syr[0], current_z + dz, syr[1], sxr[0], current_col, OGL::Plane::x);
        pltStructure->AddRectBuffer(current_z, syr[0], current_z + dz, syr[1], sxr[1], current_col, OGL::Plane::x);

        pltStructure->AddRectBuffer(current_z, sxr[0], current_z + dz, sxr[1], syr[0], current_col, OGL::Plane::y);
        pltStructure->AddRectBuffer(current_z, sxr[0], current_z + dz, sxr[1], syr[1], current_col, OGL::Plane::y);

        current_z += dz;
    }

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
