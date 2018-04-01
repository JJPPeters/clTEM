#include <utilities/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include "simulationframe.h"
#include "ui_simulationframe.h"

SimulationFrame::SimulationFrame(QWidget *parent) :
    QWidget(parent), Main(0),
    ui(new Ui::SimulationFrame)
{
    ui->setupUi(this);
}

SimulationFrame::~SimulationFrame()
{
    delete ui;
}

void SimulationFrame::updateStructureInfo(std::tuple<float, float, float, int> ranges)
{
    ui->lblWidth->setText(QString::fromStdString(Utils::numToString(std::get<0>(ranges), 2, true) + " Å"));
    ui->lblHeight->setText(QString::fromStdString(Utils::numToString(std::get<1>(ranges), 2, true) + " Å"));
    ui->lblDepth->setText(QString::fromStdString(Utils::numToString(std::get<2>(ranges), 2, true) + " Å"));
    ui->lblAtoms->setText(QString::fromStdString(Utils::numToString(std::get<3>(ranges))));
}

void SimulationFrame::updateResolutionInfo(float pixScale, float invScale, float invMax)
{
    //TODO: find way of stopping the program using scientific notation, particularly for the inverse scale

    ui->lblRealScale->setText(QString::fromStdString(Utils::numToString(pixScale, 2) + " Å"));

    ui->lblInverseScale->setText(QString::fromStdString(Utils::numToString(invScale*1000, 2) + "&times;10<sup>3</sup> Å<sup>-1</sup>"));

    ui->lblMaxInverse->setText(QString::fromStdString(Utils::numToString(invMax, 2) + " mrad"));
}

void SimulationFrame::on_cmbResolution_currentIndexChanged(const QString &selection)
{
    // convert text to a number
    int res = selection.toInt();

    emit resolutionSet(res);
}

void SimulationFrame::on_chkFull3D_toggled(bool checked)
{
    if (Main == 0)
        throw std::runtime_error("Error connecting simulation frame to main window.");

    if(checked && ui->chkFiniteDiff->isChecked())
        ui->chkFiniteDiff->setChecked(false);

    Main->Manager->setFull3d(ui->chkFull3D->isChecked());
    Main->Manager->setFull3d(ui->chkFiniteDiff->isChecked());
}

void SimulationFrame::on_chkFiniteDiff_toggled(bool checked)
{
    if (Main == 0)
        throw std::runtime_error("Error connecting simulation frame to main window.");

    if(checked && ui->chkFull3D->isChecked())
        ui->chkFull3D->setChecked(false);

    Main->Manager->setFull3d(ui->chkFull3D->isChecked());
    Main->Manager->setFull3d(ui->chkFiniteDiff->isChecked());
}

void SimulationFrame::on_btnSimArea_clicked()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting simulation frame to main window.");

    SimAreaDialog* myDialog = new SimAreaDialog(this, Main->Manager);

    connect(myDialog->getFrame(), SIGNAL(resolutionChanged(QString)), this, SLOT(setResolutionText(QString)));
    connect(myDialog->getFrame(), SIGNAL(modeChanged(int)), Main, SLOT(set_active_mode(int)));
    connect(myDialog->getFrame(), SIGNAL(updateMainCbed()), Main->getCbedFrame(), SLOT(update_text_boxes()));
    connect(myDialog->getFrame(), SIGNAL(updateMainStem()), Main->getStemFrame(), SLOT(updateScaleLabels()));

    myDialog->exec();

    //disconnect signals?
}

void SimulationFrame::updateLimits()
{
    // I could just send the signal one deeper, but I really cba. lets just get out mainwindow pointer instead...
    if (Main == 0)
        throw std::runtime_error("Error connecting simulation frame to main window.");

    auto r = Main->getSimulationArea();

//    Main->getStemArea()->setRangeXInsideSim(r);
//    Main->getStemArea()->setRangeYInsideSim(r);
    Main->getStemArea()->setRangeX(r->getLimitsX()[0], r->getLimitsX()[1]);
    Main->getStemArea()->setRangeY(r->getLimitsY()[0], r->getLimitsY()[1]);

    Main->updateScales();
    Main->updateRanges();
}

void SimulationFrame::assignMainWindow(MainWindow *m)
{
    Main = m;
    ui->chkFiniteDiff->setChecked(Main->Manager->isFiniteDifference());
    ui->chkFull3D->setChecked(Main->Manager->isFull3d());
}

void SimulationFrame::setResolutionIndex(int ind) {
    ui->cmbResolution->setCurrentIndex(ind);
}

void SimulationFrame::setResolutionText(QString text) {
    int ind = ui->cmbResolution->findText( text );
    ui->cmbResolution->setCurrentIndex(ind);
}
