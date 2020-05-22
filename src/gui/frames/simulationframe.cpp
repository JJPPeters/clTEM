#include <utilities/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include "simulationframe.h"
#include "ui_simulationframe.h"
#include <utils/stringutils.h>

SimulationFrame::SimulationFrame(QWidget *parent) :
    QWidget(parent), Main(nullptr),
    ui(new Ui::SimulationFrame)
{
    ui->setupUi(this);
}

SimulationFrame::~SimulationFrame()
{
    delete ui;
}

void SimulationFrame::updateStructureInfo(std::tuple<double, double, double, int> ranges)
{
    ui->lblWidth->setText(Utils_Qt::numToQString(std::get<0>(ranges), 2, true) + " Å");
    ui->lblHeight->setText(Utils_Qt::numToQString(std::get<1>(ranges), 2, true) + " Å");
    ui->lblDepth->setText(Utils_Qt::numToQString(std::get<2>(ranges), 2, true) + " Å");
    ui->lblAtoms->setText(Utils_Qt::numToQString(std::get<3>(ranges)));
}

void SimulationFrame::updateResolutionInfo(double pixScale, double invScale, double invMax)
{
    //TODO: find way of stopping the program using scientific notation, particularly for the inverse scale

    ui->lblRealScale->setText(Utils_Qt::numToQString(pixScale, 2) + " Å");

    ui->lblInverseScale->setText(Utils_Qt::numToQString(invScale*1000, 2) + "&times;10<sup>-3</sup> Å<sup>-1</sup>");

    ui->lblMaxInverse->setText(Utils_Qt::numToQString(invMax, 2) + " mrad");
}

void SimulationFrame::on_cmbResolution_currentIndexChanged(const QString &selection)
{
    // convert text to a number
    int res = selection.toInt();

    emit resolutionSet(res);
}

void SimulationFrame::on_chkFull3D_toggled(bool checked)
{
    if (Main == nullptr)
        throw std::runtime_error("Error connecting simulation frame to main window.");

    Main->Manager->setFull3dEnabled(ui->chkFull3D->isChecked());
}

void SimulationFrame::on_btnSimArea_clicked()
{
    if (Main == nullptr)
        throw std::runtime_error("Error connecting simulation frame to main window.");

    // update the manager from the ui
    Main->updateManagerFromGui();

    SimAreaDialog* myDialog = new SimAreaDialog(nullptr, Main->Manager);

    connect(myDialog->getFrame(), &AreaLayoutFrame::resolutionChanged, this, &SimulationFrame::setResolutionText);
    connect(myDialog->getFrame(), &AreaLayoutFrame::modeChanged, Main, &MainWindow::set_active_mode);
    connect(myDialog->getFrame(), &AreaLayoutFrame::updateMainCbed, Main->getCbedFrame(), &CbedFrame::updateTextBoxes);
    connect(myDialog->getFrame(), &AreaLayoutFrame::updateMainStem, Main->getStemFrame(), &StemFrame::updateScaleLabels);
    connect(myDialog->getFrame(), &AreaLayoutFrame::areaChanged, Main, &MainWindow::updateScales);

    myDialog->exec();
}

//void SimulationFrame::updateLimits()
//{
//    // I could just send the signal one deeper, but I really cba. lets just get out mainwindow pointer instead...
//    if (Main == nullptr)
//        throw std::runtime_error("Error connecting simulation frame to main window.");
//
//    auto r = Main->simulationArea();
//
//    Main->stemArea()->setRangeX(r->getLimitsX()[0], r->getLimitsX()[1]);
//    Main->stemArea()->setRangeY(r->getLimitsY()[0], r->getLimitsY()[1]);
//
//    Main->updateScales();
//    Main->updateRanges();
//}

void SimulationFrame::assignMainWindow(MainWindow *m)
{
    Main = m;
    ui->chkFull3D->setChecked(Main->Manager->full3dEnabled());
}

void SimulationFrame::setResolutionIndex(int ind) {
    ui->cmbResolution->setCurrentIndex(ind);
}

void SimulationFrame::setResolutionText(QString text) {
    int ind = ui->cmbResolution->findText( text );
    ind += (ind == -1);
    ui->cmbResolution->setCurrentIndex(ind);
}
