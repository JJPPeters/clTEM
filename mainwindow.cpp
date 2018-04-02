#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QStandardPaths>
//#include <QWidget>
//#include <QFileDialog>
//#include <QtWidgets/QMessageBox>
#include <controls/imagetab.h>
#include <controls/statuslayout.h>

#include <dialogs/settings/settingsdialog.h>
#include <simulation/kernels.h>
#include <utils/stringutils.h>
#include <simulation/structure/structureparameters.h>
//#include <QtWidgets/QProgressBar>
//
//#include "dialogs/settings/settingsdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    Manager(new SimulationManager())
{
    // test opencl
    try {
        ClManager::getDeviceList();
    }
    catch (const std::exception& e) {
        // TODO: later, this shouldn't exit but should just disable everything relevant
        throw std::runtime_error("Failed to get any OpenCl devices. Exiting...");
    }

    qRegisterMetaType< std::map<std::string, Image<float>> >( "std::map<std::string, Image<float>>" );

    QCoreApplication::setOrganizationName("PetersSoft");
    QCoreApplication::setApplicationName("TEM++");

    QSettings settings;
    if (!settings.contains("dialog/currentPath"))
        settings.setValue("dialog/currentPath", QStandardPaths::HomeLocation);
    if (!settings.contains("dialog/currentSavePath"))
        settings.setValue("dialog/currentSavePath", QStandardPaths::HomeLocation);

    loadSavedOpenClSettings();

    ui->setupUi(this);

    setWindowTitle("clTEM");

    ImageTab* Img = new ImageTab(ui->twReal, "Image", TabType::CTEM);
    ImageTab* EwAmp = new ImageTab(ui->twReal, "EW A", TabType::CTEM);
    ImageTab* EwAng = new ImageTab(ui->twReal, "EW θ", TabType::CTEM);

    ImageTab* Diff = new ImageTab(ui->twRecip, "Diffraction", TabType::DIFF);

    StatusBar = new StatusLayout();

    ui->statusBar->addWidget(StatusBar, 100);

    ui->twReal->addTab(Img, QString::fromStdString(Img->getTabName()));
    ui->twReal->addTab(EwAmp, QString::fromStdString(EwAmp->getTabName()));
    ui->twReal->addTab(EwAng, QString::fromStdString(EwAng->getTabName()));

    ui->twRecip->addTab(Diff, QString::fromStdString(Diff->getTabName()));

    // this is required so the frame and then dialog can access the current aberrations at any time
    // could be avoided but I've used Qt designer with the .ui files and so on
    ui->tAberr->assignMainWindow(this);
    ui->tSim->assignMainWindow(this);
    ui->tStem->assignMainWindow(this);
    ui->tCbed->assignMainWindow(this);

    ui->tStem->updateScaleLabels();

    auto p = Manager->getMicroscopeParams();
    ui->tAberr->updateTextBoxes();

    connect(ui->tSim, SIGNAL(resolutionSet(int)), this, SLOT(resolution_changed(int)));

    connect(ui->tStem, SIGNAL(startSim()), this, SLOT(on_actionSimulate_EW_triggered()));
    connect(ui->tTem, SIGNAL(startSim()), this, SLOT(on_actionSimulate_EW_triggered()));
    connect(ui->tCbed, SIGNAL(startSim()), this, SLOT(on_actionSimulate_EW_triggered()));
    connect(ui->twMode, SIGNAL(currentChanged(int)), this, SLOT(on_twMode_currentChanged(int)));

    connect(ui->tTem, SIGNAL(stopSim()), this, SLOT(cancel_simulation()));
    connect(ui->tCbed, SIGNAL(stopSim()), this, SLOT(cancel_simulation()));
    connect(ui->tStem, SIGNAL(stopSim()), this, SLOT(cancel_simulation()));

    // for the image simulation
    connect(ui->tTem, SIGNAL(setSimImage(bool)), this, SLOT(set_sim_image(bool)));

    connect(ui->tTem, SIGNAL(setCtemCrop(bool)), this, SLOT(set_ctem_crop(bool)));

    ui->tSim->setResolutionIndex(0);
    ui->tTem->setCropCheck(true);
    ui->tTem->setSimImageCheck(true);

    loadExternalSources();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QSettings settings;

    QString fileName = QFileDialog::getOpenFileName(this, "Open File", settings.value("dialog/currentPath").toString(), "All supported (*.xyz);; XYZ (*.xyz)");

    if (fileName.isNull())
        return;

    QFileInfo temp_file(fileName);

    settings.setValue("dialog/currentPath", temp_file.path());

    if (temp_file.suffix() == "xyz")
        Manager->setStructure(fileName.toStdString());

    auto ar = Manager->getSimulationArea();
    bool changed = false;
    changed &= Manager->getStemArea()->setRangeX(ar->getLimitsX()[0], ar->getLimitsX()[1]);
    changed &= Manager->getStemArea()->setRangeY(ar->getLimitsY()[0], ar->getLimitsY()[1]);

    // TODO: so I want a message here?
//    if(changed)
//        QMessageBox::warning(this, tr("Area conflict"), tr("Warning STEM area has been modified"), QMessageBox::Ok);

    // update frames to show limits
    updateRanges();

    updateScales();

    ui->tStem->updateScaleLabels();
}

void MainWindow::on_actionOpenCL_triggered()
{
    // TODO: later will want to pass the full tuple (performance factors and all)
    OpenClDialog *myDialog = new OpenClDialog(this, std::get<0>(Devices));

    myDialog->exec();

    auto chosen = myDialog->getChosenDevices();

//    Manager->setOpenClDevices(chosen);
    Devices = chosen;

    // remove all current device entries in the settings and reset them
    QSettings settings;
    settings.remove("opencl");
    int counter = 0;
    for (auto& dev : std::get<0>(Devices))
    {
        settings.setValue("opencl/" + QString::number(counter) + "/platform", dev.GetPlatformNumber());
        settings.setValue("opencl/" + QString::number(counter) + "/device", dev.GetDeviceNumber());
        settings.setValue("opencl/" + QString::number(counter) + "/platform_name", QString::fromStdString(dev.GetPlatformName()));
        settings.setValue("opencl/" + QString::number(counter) + "/device_name", QString::fromStdString(dev.GetDeviceName()));
        ++counter;
    }
}

void MainWindow::resolution_changed(int resolution)
{
    // set resolution in manager
    Manager->setResolution(resolution);

    updateScales();
}

void MainWindow::updateScales()
{
    if (!Manager->haveStructure() || !Manager->haveResolution())
        return;

    ui->tSim->updateResolutionInfo(Manager->getRealScale(), Manager->getInverseScale(), Manager->getInverseMaxAngle());
    ui->tSim->updateStructureInfo(Manager->getSimRanges());
}

void MainWindow::updateRanges()
{
    if (!Manager->haveStructure())
        return;

    ui->tSim->updateStructureInfo(Manager->getSimRanges());;
}

void MainWindow::setDetectors()
{
    //Manager->setDetectors(d);

    // this adds any detectors that are new
    for (auto d : Manager->getDetectors())
    {
        bool exists = false;
        for (int i = 0; i < ui->twReal->count() && !exists; ++i)
            if (ui->twReal->tabText(i).toStdString() == d.name)
                exists = true;

        if (!exists)
        {
            ImageTab* tb = new ImageTab(ui->twReal, d.name, TabType::STEM);
            ui->twReal->addTab(tb, QString::fromStdString(tb->getTabName()));
        }
    }

    // this removes tabs that no londer exists
    // remember we have 4 'static' tabs to keep
    for (int i = ui->twReal->count()-1; i >= 0; --i) // loop backwards so we don't have to deal with the trab indices changing
    {
        ImageTab* t = static_cast<ImageTab*>(ui->twReal->widget(i));

        if (t->getType() == TabType::STEM)
        {
            bool exists = false;
            for (auto d : Manager->getDetectors())
            {
                if (ui->twReal->tabText(i).toStdString() == d.name)
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
                ui->twReal->removeTab(i);
        }
    }
}

void MainWindow::on_twMode_currentChanged(int index)
{
    if (index == 0)
        Manager->setMode(SimulationMode::CTEM);
    else if (index == 1)
        Manager->setMode(SimulationMode::STEM);
    else if (index == 2)
        Manager->setMode(SimulationMode::CBED);

    updateScales();
}

void MainWindow::updateSlicesProgress(float prog)
{
    QMutexLocker locker(&Progress_Mutex);
    emit sliceProgressUpdated(prog);
}

void MainWindow::updateTotalProgress(float prog)
{
    QMutexLocker locker(&Progress_Mutex);
    emit totalProgressUpdated(prog);
}

void MainWindow::updateImages(std::map<std::string, Image<float>> ims)
{
    QMutexLocker locker(&Image_Mutex);
    emit imagesReturned(ims);
}

void MainWindow::on_actionSimulate_EW_triggered(bool do_image)
{
    // Start by stopping the user attempting to run the simulation again
    setUiActive(false);

    // Set some variables that aren't auto updates

    // Sort out TDS bits
    if (Manager->getMode() == SimulationMode::CBED)
    {
        Manager->setTdsEnabled(ui->tCbed->isTdsEnabled());
        Manager->setTdsRuns(ui->tCbed->getTdsRuns());
    }
    else if (Manager->getMode() == SimulationMode::STEM)
    {
        Manager->setTdsEnabled(ui->tStem->isTdsEnabled());
        Manager->setTdsRuns(ui->tStem->getTdsRuns());
    }

    // update aberrations from the main tab
    // TODO: get full aberrations from the dialg too
    ui->tAberr->updateAberrations();

    // test we have everything we need
    try
    {
        checkSimulationPrerequisites();
    }
    catch (const std::runtime_error& e)
    {
        QMessageBox msgBox;
        msgBox.setText("Error:");
        msgBox.setInformativeText(e.what());
//        msgBox.setDetailedText(e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        setUiActive(true);
        return;
    }

    std::vector<std::shared_ptr<SimulationManager>> man_list; //why is this a vector?

    auto sliceRep = std::bind(&MainWindow::updateSlicesProgress, this, std::placeholders::_1);
    Manager->setProgressReporterFunc(sliceRep);

    auto imageRet = std::bind(&MainWindow::updateImages, this, std::placeholders::_1);
    Manager->setImageReturnFunc(imageRet);

    auto mp = Manager->getMicroscopeParams();

    auto temp = std::make_shared<SimulationManager>(*Manager);

    man_list.push_back(temp);

    std::vector<clDevice> &d = std::get<0>(Devices);

    SimThread = std::shared_ptr<SimulationThread>(new SimulationThread(man_list, d));

    connect(this, &MainWindow::sliceProgressUpdated, this, &MainWindow::sliceProgressChanged);
    connect(this, &MainWindow::totalProgressUpdated, this, &MainWindow::totalProgressChanged);
    connect(this, &MainWindow::imagesReturned, this, &MainWindow::imagesChanged);

    SimThread->start();
}

void MainWindow::sliceProgressChanged(float prog)
{
    StatusBar->setSliceProgress(prog);
}

void MainWindow::totalProgressChanged(float prog)
{
    StatusBar->setTotalProgress(prog);
}

void MainWindow::imagesChanged(std::map<std::string, Image<float>> ims)
{
    // we've been given a list of images, got to display them now....
    for (auto const& i : ims)
    {
        std::string name = i.first;
        auto im = i.second;
        // Currently assumes the positions of all the tabs

        if (name == "EW_A")
        {
            int n = ui->twReal->count();
            for (int j = 0; j < n; ++j)
            {
                ImageTab *tab = (ImageTab *) ui->twReal->widget(j);
                if (tab->getTabName() == "EW A")
                    tab->getPlot()->SetImageTemplate(im);
            }
        }
        else if (name == "Image")
        {
            int n = ui->twReal->count();
            for (int j = 0; j < n; ++j)
            {
                ImageTab *tab = (ImageTab *) ui->twReal->widget(j);
                if (tab->getTabName() == "Image")
                    tab->getPlot()->SetImageTemplate(im);
            }
        }
        else if (name == "EW_T")
        {
            int n = ui->twReal->count();
            for (int j = 0; j < n; ++j)
            {
                ImageTab *tab = (ImageTab *) ui->twReal->widget(j);
                if (tab->getTabName() == "EW θ")
                    tab->getPlot()->SetImageTemplate(im);
            }
        }
        else if (name == "Diff")
        {
            int n = ui->twRecip->count();
            for (int j = 0; j < n; ++j)
            {
                ImageTab *tab = (ImageTab *) ui->twRecip->widget(j);
                if (tab->getTabName() == "Diffraction")
                    tab->getPlot()->SetImageTemplate(im, IntensityScale::Log);
            }
        }
        else
        {
            // Handle general cases (for STEM really)
            int n = ui->twReal->count();
            for (int j = 0; j < n; ++j)
            {
                ImageTab *tab = (ImageTab *) ui->twReal->widget(j);
                if (tab->getTabName() == name)
                    tab->getPlot()->SetImageTemplate(im);
            }
        }
    }

    simulationComplete();
}

void MainWindow::setUiActive(bool active)
{
    //disable things the user shouldn't be able to access whilst a simulation is running
    ui->tTem->setActive(active);
    ui->tCbed->setActive(active);
    ui->tStem->setActive(active);
}

void MainWindow::loadSavedOpenClSettings()
{
    // SET THE SAVED OPENCL DEVICES (IF IT EXISTS STILL)
    // seemingly can't compare device id, so compare platform/device numbers and name
    // these might change with hardware changes, but not enough to be annoying (I think)
    QSettings settings;
    settings.beginGroup("opencl");
    QStringList devs = settings.childGroups();
    std::vector<clDevice> dev_list;
    auto present_devs = ClManager::getDeviceList();
    for (int i = 0; i < devs.size(); ++i)
    {
        int dev_num = settings.value(devs[i] + "/device").toInt();
        int plat_num = settings.value(devs[i] + "/platform").toInt();
        std::string dev_name = settings.value(devs[i] + "/device_name").toString().toStdString();
        std::string plat_name = settings.value(devs[i] + "/platform_name").toString().toStdString();

        for (auto d : present_devs)
        {
            if (d.GetDeviceName() == dev_name && d.GetPlatformName() == plat_name && d.GetDeviceNumber() == dev_num &&
                d.GetPlatformNumber() == plat_num)
                dev_list.push_back(d);
        }
    }
    std::vector<float> perfs(dev_list.size(), 1.0f); //TODO: implement this properly later...
    Devices = std::tuple<std::vector<clDevice>, std::vector<float>>(dev_list, perfs);
    settings.endGroup();
}

bool MainWindow::checkSimulationPrerequisites()
{
    std::vector<std::string> errorList;

    if(std::get<0>(Devices).size() <= 0)
        errorList.push_back("No OpenCL devices selected.");

    if(!Manager->getStructure())
        errorList.push_back("No structure loaded.");

    if(!Manager->haveResolution())
        errorList.push_back("No valid simulation resolution set.");

    auto mp = Manager->getMicroscopeParams();

    if(mp->Voltage <= 0)
        errorList.push_back("Voltage must be a non-zero positive number.");
    if(mp->Aperture <= 0)
        errorList.push_back("Aperture must be a non-zero positive number.");

    // TODO: check beta (alpha) and delta?

    // TODO: check TDS entries

    // TODO: CBED position in simulation area

    // TODO: STEM area in simulation area

    // TODO: STEM detectors exist

    // TODO: dose sim for TEM checks

    // TODO: warnings option (stem detector radius checks...

    if (errorList.size() > 0) {
        std::string final = "";
        for (const auto &err : errorList) {
            final += err + "\n";
        }
        throw std::runtime_error(final);
    }

}

void MainWindow::simulationComplete()
{
    setUiActive(true);
}

void MainWindow::cancel_simulation()
{
    // stop the simulation
    if (SimThread)
        SimThread->cancelSimulation();

    // reset the ui
    disconnect(this, SIGNAL(sliceProgressUpdated()), 0, 0);
    disconnect(this, SIGNAL(totalProgressUpdated()), 0, 0);
    disconnect(this, SIGNAL(imagesReturned()), 0, 0);

    setUiActive(true);

    sliceProgressChanged(0.0f);
    totalProgressChanged(0.0f);
}

void MainWindow::loadExternalSources()
{
    // Populate the kernels from files...
    Kernels::atom_sort = Utils::kernelToChar("atom_sort.cl");
    Kernels::floatSumReductionsource2 = Utils::kernelToChar("sum_reduction.cl");
    Kernels::BandLimitSource = Utils::kernelToChar("low_pass.cl");
    Kernels::fftShiftSource = Utils::kernelToChar("post_fft_shift.cl");
    Kernels::opt2source = Utils::kernelToChar("potential_full_3d.cl");
    Kernels::fd2source = Utils::kernelToChar("potential_finite_difference.cl");
    Kernels::conv2source = Utils::kernelToChar("potential_conventional.cl");
    Kernels::propsource = Utils::kernelToChar("generate_propagator.cl");
    Kernels::multisource = Utils::kernelToChar("complex_multiply.cl");
    Kernels::gradsource = Utils::kernelToChar("grad.cl");
    Kernels::fdsource = Utils::kernelToChar("finite_difference.cl");
    Kernels::InitialiseWavefunctionSource = Utils::kernelToChar("initialise_plane.cl");
    Kernels::imagingKernelSource = Utils::kernelToChar("generate_tem_image.cl");
    Kernels::InitialiseSTEMWavefunctionSourceTest = Utils::kernelToChar("initialise_probe.cl");
    Kernels::floatabsbandPassSource = Utils::kernelToChar("band_pass.cl");
    Kernels::SqAbsSource = Utils::kernelToChar("square_absolute.cl");

    // load parameters (kirkland for now)
    std::vector<float> params = Utils::paramsToVector("kirkland.dat");
    StructureParameters::setParams(params);

    // TODO: load DQE, NQE for the CTEM simulation
}

void MainWindow::set_active_mode(int mode)
{
    ui->twMode->setCurrentIndex(mode);
}

SimulationFrame *MainWindow::getSimulationFrame() {return ui->tSim;}
StemFrame *MainWindow::getStemFrame() {return ui->tStem;}
CbedFrame *MainWindow::getCbedFrame() {return ui->tCbed;}

void MainWindow::set_ctem_crop(bool state) {
    // do the real images
    int n = ui->twReal->count();
    for (int j = 0; j < n; ++j) {
        ImageTab *tab = (ImageTab *) ui->twReal->widget(j);
        if (tab->getTabName() == "EW A")
            tab->getPlot()->setCropImage(state, true, false);
        else if (tab->getTabName() == "EW θ")
            tab->getPlot()->setCropImage(state, true, false);
        else if (tab->getTabName() == "Image")
            tab->getPlot()->setCropImage(state, true, false);
    }
}

void MainWindow::set_sim_image(bool state) {
    Manager->setSimulateCtemImage(state);
}
