#include <memory>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QStandardPaths>
#include <QtCore/QDir>
#include <controls/imagetab.h>
#include <controls/statuslayout.h>

#include <dialogs/settings/settingsdialog.h>
#include <kernels.h>
#include <utils/stringutils.h>
#include <structure/structureparameters.h>
#include <ccdparams.h>
#include <utilities/fileio.h>
#include <utilities/jsonutils.h>
#include <frames/aberrationframe.h>
#include <cif/supercell.h>

#include <variant>

MainWindow::MainWindow(QWidget *parent) :
    BorderlessWindow(parent),
    ui(new Ui::MainWindow)
{
    // opencl test has been moved to main.cpp

    // register types for our image returns!!
    qRegisterMetaType< std::map<std::string, Image<float>> >( "std::map<std::string, Image<float>>" );
    qRegisterMetaType< SimulationManager >( "SimulationManager" );

    QSettings settings;
    if (!settings.contains("dialog/currentPath"))
        settings.setValue("dialog/currentPath", QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    if (!settings.contains("dialog/currentSavePath"))
        settings.setValue("dialog/currentSavePath", QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

    Manager = std::make_shared<SimulationManager>();

    std::string exe_path = qApp->applicationDirPath().toStdString();

    // try loading default settings from the config location
    on_actionImport_default_triggered(true);

    loadSavedOpenClSettings();

    ui->setupUi(this);

    setWindowTitle("clTEM");

#ifndef _WIN32
    // Hide our theme menu options if not on windows
    ui->actionTheme->setEnabled(false);
    ui->actionTheme->setVisible(false);
#endif

    ImageTab* Img = new ImageTab(ui->twReal, "Image", TabType::CTEM);
    ImageTab* EwAmp = new ImageTab(ui->twReal, "EW", TabType::CTEM, true);
    ImageTab* Diff = new ImageTab(ui->twRecip, "Diffraction", TabType::DIFF);

    StatusBar = new StatusLayout();

    ui->statusBar->addWidget(StatusBar, 100);

    ui->twReal->addTab(Img, QString::fromStdString(Img->getTabName()));
    ui->twReal->addTab(EwAmp, QString::fromStdString(EwAmp->getTabName()));

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

    connect(ui->tSim, &SimulationFrame::resolutionSet, this, &MainWindow::resolution_changed);

    connect(ui->tStem, &StemFrame::startSim, this, &MainWindow::on_actionSimulate_EW_triggered);
    connect(ui->tTem, &TemFrame::startSim, this, &MainWindow::on_actionSimulate_EW_triggered);
    connect(ui->tCbed, &CbedFrame::startSim, this, &MainWindow::on_actionSimulate_EW_triggered);
    connect(ui->twMode, &QTabWidget::currentChanged, this, &MainWindow::on_twMode_currentChanged);

    connect(ui->tTem, &TemFrame::stopSim, this, &MainWindow::cancel_simulation);
    connect(ui->tCbed, &CbedFrame::stopSim, this, &MainWindow::cancel_simulation);
    connect(ui->tStem, &StemFrame::stopSim, this, &MainWindow::cancel_simulation);

    connect(ui->tTem, &TemFrame::setCtemCrop, this, &MainWindow::set_ctem_crop);

    connect(this, &MainWindow::sliceProgressUpdated, this, &MainWindow::sliceProgressChanged);
    connect(this, &MainWindow::totalProgressUpdated, this, &MainWindow::totalProgressChanged);
    connect(this, &MainWindow::imagesReturned, this, &MainWindow::imagesChanged);

    int n = ui->twReal->count();
    for (int j = 0; j < n; ++j) {
        auto *tab = (ImageTab *) ui->twReal->widget(j);
        connect(tab, &ImageTab::saveDataActivated, this, &MainWindow::saveTiff);
        connect(tab, &ImageTab::saveImageActivated, this, &MainWindow::saveBmp);
    }
    n = ui->twRecip->count();
    for (int j = 0; j < n; ++j) {
        auto *tab = (ImageTab *) ui->twRecip->widget(j);
        connect(tab, &ImageTab::saveDataActivated, this, &MainWindow::saveTiff);
        connect(tab, &ImageTab::saveImageActivated, this, &MainWindow::saveBmp);
    }

    loadExternalSources();

    updateGuiFromManager();
    ui->tTem->setCropCheck( true );
}

MainWindow::~MainWindow()
{
    if(SimThread)
        SimThread->cancelSimulation();

    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QSettings settings;

    QString fileName = QFileDialog::getOpenFileName(this, "Open file", settings.value("dialog/currentPath").toString(), "All supported (*.xyz *.cif);; XYZ (*.xyz);; CIF (*.xyz)");

    if (fileName.isNull())
        return;

    QFileInfo temp_file(fileName);

    settings.setValue("dialog/currentPath", temp_file.path());

    try {
        // TODO: there needs to be an extra dialog step for cif format
        if (temp_file.suffix() == "xyz")
            Manager->setStructure(fileName.toStdString());
        else if (temp_file.suffix() == "cif") {
            // open dialog to open cif
            // read the cif now so we can pass it to the dialog

            auto cif = CIF::CIFReader(fileName.toStdString());
            auto info = std::make_shared<CIF::SuperCellInfo>();

            auto myDialog = new CifCreatorDialog(this, cif, info);
            auto result = myDialog->exec();

            if(result == QDialog::Accepted)
                Manager->setStructure(fileName.toStdString(), *info);
        }
        else
            throw std::runtime_error("." + temp_file.suffix().toStdString() + " is not a supported file format");
    } catch (const std::exception &e) {
        CLOG(ERROR, "gui") << "Could not open file: " << e.what() << ".";
        QMessageBox msgBox(this);
        msgBox.setText("Error:");
        msgBox.setInformativeText(e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setMinimumSize(160, 125);
        msgBox.exec();
        return;
    }
    // update frames to show limits
    updateRanges();

    updateScales();

    ui->tStem->updateScaleLabels();

    StatusBar->setFileLabel(fileName);
}

void MainWindow::on_actionOpenCL_triggered()
{
    // TODO: later will want to pass the full tuple (performance factors and all)
    OpenClDialog *myDialog = new OpenClDialog(this, Devices);

    myDialog->exec();

    // remove all current device entries in the settings and reset them
    QSettings settings;
    settings.remove("opencl");
    int counter = 0;
    for (auto& dev : Devices)
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

            // connect the slots up
            connect(tb, &ImageTab::saveDataActivated, this, &MainWindow::saveTiff);
            connect(tb, &ImageTab::saveImageActivated, this, &MainWindow::saveBmp);
        }
    }

    // this removes tabs that no longer exists
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

void MainWindow::updateImages(SimulationManager sm)
{
    QMutexLocker locker(&Image_Mutex);
    // this will be running in a different thread
    emit imagesReturned(sm);
}

void MainWindow::on_actionSimulate_EW_triggered()
{
    // Start by stopping the user attempting to run the simulation again
    setUiActive(false);

    StatusBar->setSliceProgress(0.f);
    StatusBar->setTotalProgress(0.f);

    // Set some variables that aren't auto updates

    updateManagerFromGui();

    // test we have everything we need
    try {
        Utils::checkSimulationPrerequisites(Manager, Devices);
    }
    catch (const std::runtime_error& e) {
        CLOG(WARNING, "gui") << "Simulation prerequisites not met: " << e.what();
        QMessageBox msgBox(this);
        msgBox.setText("Error:");
        msgBox.setInformativeText(e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setMinimumSize(160, 125);
        msgBox.exec();
        setUiActive(true);
        return;
    }

    std::vector<std::shared_ptr<SimulationManager>> man_list; //why is this a vector?

    auto sliceRep = std::bind(&MainWindow::updateSlicesProgress, this, std::placeholders::_1);
    Manager->setProgressSliceReporterFunc(sliceRep);

    auto totalRep = std::bind(&MainWindow::updateTotalProgress, this, std::placeholders::_1);
    Manager->setProgressTotalReporterFunc(totalRep);

    auto imageRet = std::bind(&MainWindow::updateImages, this, std::placeholders::_1);
    Manager->setImageReturnFunc(imageRet);

    auto temp = std::make_shared<SimulationManager>(*Manager);

    man_list.push_back(temp);

    std::vector<clDevice> &d = Devices;

    SimThread = std::make_shared<SimulationThread>(man_list, d);

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

void MainWindow::imagesChanged(SimulationManager sm)
{

    auto ims = sm.getImages();

    if (ims.empty()) {
        simulationFailed();
    }

    nlohmann::json settings = JSONUtils::BasicManagerToJson(sm);
    settings["filename"] = sm.getStructure()->getFileName();

    // we've been given a list of images, got to display them now....
    for (auto const& i : ims)
    {
        std::string name = i.first;
        auto im = i.second;
        // Currently assumes the positions of all the tabs

        if (name == "EW")
        {
            int n = ui->twReal->count();
            for (int j = 0; j < n; ++j)
            {
                ImageTab *tab = (ImageTab *) ui->twReal->widget(j);
                if (tab->getTabName() == "EW") {
                    settings["microscope"].erase("aberrations");
                    settings["microscope"].erase("alpha");
                    settings["microscope"].erase("delta");

                    // convert our float data to complex
                    std::vector<std::complex<float>> comp_data(im.height*im.width);
                    for (int i = 0; i < comp_data.size(); ++i)
                        comp_data[i] = std::complex<float>(im.data[2*i], im.data[2*i+1]);
                    Image<std::complex<float>> comp_im(im.width, im.height, comp_data, im.pad_t, im.pad_l, im.pad_b, im.pad_r);

                    double lx = sm.getPaddedSimLimitsX()[0];
                    double ly = sm.getPaddedSimLimitsY()[0];
                    double sc = sm.getRealScale();
                    tab->setPlotWithComplexData(comp_im, "Å", sc, sc, lx, ly, settings);
                }
            }
        }
        else if (name == "Image")
        {
            int n = ui->twReal->count();
            for (int j = 0; j < n; ++j)
            {
                ImageTab *tab = (ImageTab *) ui->twReal->widget(j);
                if (tab->getTabName() == "Image") {
                    double lx = sm.getPaddedSimLimitsX()[0];
                    double ly = sm.getPaddedSimLimitsY()[0];
                    double sc = sm.getRealScale();
                    tab->setPlotWithData(im, "Å", sc, sc, lx, ly, settings);
                }
            }
        }
        else if (name == "Diff")
        {
            int n = ui->twRecip->count();
            for (int j = 0; j < n; ++j)
            {
                ImageTab *tab = (ImageTab *) ui->twRecip->widget(j);
                if (tab->getTabName() == "Diffraction") {
                    settings["microscope"].erase("aberrations");
                    settings["microscope"].erase("alpha");
                    settings["microscope"].erase("delta");
                    double sc;
                    QString unit;
                    if (sm.getMode() == SimulationMode::CBED) {
                        sc = sm.getInverseScaleAngle();
                        unit = "mrad";
                    } else {
                        sc = sm.getInverseScale();
                        unit = "A⁻¹";
                    }
                    tab->setPlotWithData(im, unit, sc, sc, 0.0, 0.0, settings, IntensityScale::Log, ZeroPosition::Centre);
                }
            }
        }
        else
        {
            // Handle general cases (for STEM really)
            int n = ui->twReal->count();
            for (int j = 0; j < n; ++j)
            {
                ImageTab *tab = (ImageTab *) ui->twReal->widget(j);
                if (tab->getTabName() == name) {
                    // add the specific detector info here!
                    for (auto d : sm.getDetectors())
                        if (d.name == name)
                            settings["stem"]["detectors"][d.name] = JSONUtils::stemDetectorToJson(d);
                    settings["microscope"].erase("alpha");
                    settings["microscope"].erase("delta");

                    double lx = sm.getStemArea()->getRawLimitsX()[0];
                    double ly = sm.getStemArea()->getRawLimitsY()[0];
                    double scx = sm.getStemArea()->getScaleX();
                    double scy = sm.getStemArea()->getScaleY();

                    tab->setPlotWithData(im, "Å", scx, scy, lx, ly, settings);
                }
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
    Devices = std::vector<clDevice>(dev_list);
    settings.endGroup();
}

void MainWindow::simulationComplete() {
    setUiActive(true);

    if (SimThread) {
        SimThread->cancelSimulation();
    }
}

void MainWindow::simulationFailed()
{
    // reset our gui
    simulationComplete();

    // set an error message
    QMessageBox msgBox(this);
    msgBox.setText("Error:");
    msgBox.setInformativeText("Problem running simulation (see log file)");
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setMinimumSize(160, 125);
    msgBox.exec();
}

void MainWindow::cancel_simulation()
{
    // stop the simulation
    if (SimThread)
        SimThread->cancelSimulation();

    setUiActive(true);

    sliceProgressChanged(0.0f);
    totalProgressChanged(0.0f);
}

void MainWindow::loadExternalSources()
{
    // Populate the kernels from files...
    Kernels::atom_sort = Utils_Qt::kernelToChar("atom_sort.cl");
    Kernels::floatSumReductionsource2 = Utils_Qt::kernelToChar("sum_reduction.cl");
    Kernels::BandLimitSource = Utils_Qt::kernelToChar("low_pass.cl");
    Kernels::fftShiftSource = Utils_Qt::kernelToChar("post_fft_shift.cl");
    Kernels::opt2source = Utils_Qt::kernelToChar("potential_full_3d.cl");
    Kernels::conv2source = Utils_Qt::kernelToChar("potential_conventional.cl");
    Kernels::propsource = Utils_Qt::kernelToChar("generate_propagator.cl");
    Kernels::multisource = Utils_Qt::kernelToChar("complex_multiply.cl");
    Kernels::InitialiseWavefunctionSource = Utils_Qt::kernelToChar("initialise_plane.cl");
    Kernels::imagingKernelSource = Utils_Qt::kernelToChar("generate_tem_image.cl");
    Kernels::InitialiseSTEMWavefunctionSourceTest = Utils_Qt::kernelToChar("initialise_probe.cl");
    Kernels::floatabsbandPassSource = Utils_Qt::kernelToChar("band_pass.cl");
    Kernels::SqAbsSource = Utils_Qt::kernelToChar("square_absolute.cl");
    Kernels::DqeSource = Utils_Qt::kernelToChar("dqe.cl");
    Kernels::NtfSource = Utils_Qt::kernelToChar("ntf.cl");

    // load parameters
    // get all the files in the parameters folder
    auto params_path = qApp->applicationDirPath() + "/params/";
    QDir params_dir(params_path);
    QStringList params_filt;
    params_filt << "*.dat";
    QStringList params_files = params_dir.entryList(params_filt);

    if (params_files.empty())
        throw std::runtime_error("Need at least one valid parameters file");

    for (int k = 0; k < params_files.size(); ++k) {
        unsigned int row_count;
        std::vector<float> params = Utils_Qt::paramsToVector(params_files[k].toStdString(), row_count);
        std::string p_name = params_files[k].toStdString();
        p_name.erase(p_name.find(".dat"), 4);
        StructureParameters::setParams(params, p_name, row_count);
    }

    // load DQE, NQE for the CTEM simulation

    auto ccd_path = qApp->applicationDirPath() + "/ccds/";
    QDir ccd_dir(ccd_path);
    QStringList ccd_filt;
    ccd_filt << "*.dat";
    QStringList ccd_files = ccd_dir.entryList(ccd_filt);

    std::vector<float> dqe, ntf;
    std::string name;
    for (int k = 0; k < ccd_files.size(); ++k) {
        Utils_Qt::ccdToDqeNtf(ccd_files[k].toStdString(), name, dqe, ntf);
        CCDParams::addCCD(name, dqe, ntf);
    }

    ui->tTem->populateCcdCombo(CCDParams::getNames());
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
        if (tab->getTabName() == "EW")
            tab->getPlot()->setCropImage(state, true, false);
        else if (tab->getTabName() == "Image")
            tab->getPlot()->setCropImage(state, true, false);
    }
}

void MainWindow::saveTiff() {
    auto origin = dynamic_cast<ImageTab*>(sender());

    // do the dialog stuff
    QSettings settings;
    QString filepath = QFileDialog::getSaveFileName(this, "Save data", settings.value("dialog/currentSavePath").toString(), "TIFF (*.tif)");

    if (filepath.isEmpty())
        return;

    QFileInfo temp(filepath);
    settings.setValue("dialog/currentSavePath", temp.path());
    std::string fo = filepath.toStdString(); // our image output path

    // I feel that there should be a better way for this...
    // get the filepath without the extension (if it is there)
    if (fo.substr(fo.length() - 4) == ".tif")
        fo = fo.substr(0, fo.length() - 4);

    // set up where we will get our data
    int sx, sy;
    std::vector<float> data;

    origin->getPlot()->getData(data, sx, sy); // get data
    fileio::SaveTiff<float>(fo+".tif", data, sx, sy); // save data
    nlohmann::json j_settings = origin->getSettings(); // get settings
    fileio::SaveSettingsJson(fo+".json", j_settings); // save settings
}

void MainWindow::saveBmp() {
    // csat our sender to check this is all valid and good
    auto origin = dynamic_cast<ImageTab*>(sender());

    // do the dialog stuff
    QSettings settings;
    QString filepath = QFileDialog::getSaveFileName(this, "Save image", settings.value("dialog/currentSavePath").toString(), "Bitmap (*.bmp)");

    if (filepath.isEmpty())
        return;

    QFileInfo temp(filepath);
    settings.setValue("dialog/currentSavePath", temp.path());

    std::string f = filepath.toStdString();
    // I feel that there should be a better way for this...
    if (f.substr((f.length() - 4)) != ".bmp")
        f.append(".bmp");

    // now get the data by reference
    int sx, sy;
    std::vector<float> data;
    origin->getPlot()->getData(data, sx, sy);

    // and save
    fileio::SaveBmp(f, data, sx, sy);

    // change the extension an save settings!
    f.append("n");
    f.replace(f.end()-5, f.end(), ".json");
    nlohmann::json test = origin->getSettings();
    fileio::SaveSettingsJson(f, test);
}

void MainWindow::on_actionGeneral_triggered() {
    GlobalSettingsDialog *myDialog = new GlobalSettingsDialog(this, Manager);

    myDialog->exec();
}

void MainWindow::on_actionImport_parameters_triggered() {
    // open a dialog to get the json file
    QSettings settings;
    QString fileName = QFileDialog::getOpenFileName(this, "Save parameters", settings.value("dialog/currentPath").toString(), "All supported (*.json);; JSON (*.json)");

    if (fileName.isNull())
        return;
    QFileInfo temp_file(fileName);
    settings.setValue("dialog/currentPath", temp_file.path());

    if (temp_file.suffix() != "json")
        return;

    nlohmann::json j = fileio::OpenSettingsJson(fileName.toStdString());

    SimulationManager m = JSONUtils::JsonToManager(j);
    *Manager = m;

    updateGuiFromManager();
}

void MainWindow::on_actionExport_parameters_triggered() {
    // open a dialog to get the save file
    QSettings settings;
    QString fileName = QFileDialog::getSaveFileName(this, "Save parameters", settings.value("dialog/currentSavePath").toString(), "JSON (*.json)");

    if (fileName.isNull())
        return;

    QFileInfo temp_file(fileName);
    settings.setValue("dialog/currentSavePath", temp_file.path());

    if (temp_file.suffix() != "json")
        fileName.append(".json");

    updateManagerFromGui();

    nlohmann::json j = JSONUtils::FullManagerToJson(*Manager);
    fileio::SaveSettingsJson(fileName.toStdString(), j);
}

void MainWindow::updateManagerFromGui() {
    // things to do:
    // CBED position is set when it is changed...
    // Aberrations
    // CBED/STEM TDS
    // CTEM CCD stuff

    // Sort out TDS bits
    Manager->setTdsRunsCbed(ui->tCbed->getTdsRuns());
    Manager->setTdsRunsStem(ui->tStem->getTdsRuns());

    Manager->setTdsEnabledCbed(ui->tCbed->isTdsEnabled());
    Manager->setTdsEnabledStem(ui->tStem->isTdsEnabled());

    // update aberrations from the main tab
    // aberrations in the dialog are updated when you click apply
    ui->tAberr->updateAberrations();

    // load variables for potential TEM stuff
    Manager->setCcdBinning(ui->tTem->getBinning());
    Manager->setSimulateCtemImage(ui->tTem->getSimImage());
    Manager->setCcdName(ui->tTem->getCcd());
    Manager->setCcdDose(ui->tTem->getDose());
}

void MainWindow::updateGuiFromManager() {
    // set aberrations on the panel
    ui->tAberr->updateTextBoxes();

    // set CBED stuff (position/TDS)
    ui->tCbed->update_text_boxes();

    // set STEM TDS
    ui->tStem->updateTdsText();
    ui->tStem->updateScaleLabels();

    // set CTEM CCD stuff
    ui->tTem->update_ccd_boxes(Manager);

    // update the detector tabs
    setDetectors();

    ui->tTem->setSimImageCheck( Manager->getSimulateCtemImage() );

    ui->twMode->setCurrentIndex( (int) Manager->getMode() );

    // set resolution last, this should update the structure area stuff if it needs to be
    ui->tSim->setResolution( Manager->getResolution() );
}

void MainWindow::on_actionSet_area_triggered()
{
    updateManagerFromGui();

    SimAreaDialog* myDialog = new SimAreaDialog(this, Manager);

    connect(myDialog->getFrame(), &AreaLayoutFrame::resolutionChanged, ui->tSim, &SimulationFrame::setResolutionText);
    connect(myDialog->getFrame(), &AreaLayoutFrame::modeChanged, this, &MainWindow::set_active_mode);
    connect(myDialog->getFrame(), &AreaLayoutFrame::updateMainCbed, getCbedFrame(), &CbedFrame::update_text_boxes);
    connect(myDialog->getFrame(), &AreaLayoutFrame::updateMainStem, getStemFrame(), &StemFrame::updateScaleLabels);
    connect(myDialog->getFrame(), &AreaLayoutFrame::areaChanged, this, &MainWindow::updateScales);

    myDialog->exec();
}

void MainWindow::on_actionAberrations_triggered()
{
    ui->tAberr->updateAberrations(); // here we update the current aberrations from the text boxes here so the dialog can show the same
    AberrationsDialog* myDialog = new AberrationsDialog(this, Manager->getMicroscopeParams());
    connect(myDialog, &AberrationsDialog::aberrationsChanged, ui->tAberr, &AberrationFrame::updateTextBoxes);
    myDialog->exec();
}

void MainWindow::on_actionThermal_scattering_triggered() {
    ThermalScatteringDialog* myDialog = new ThermalScatteringDialog(this, Manager);
    myDialog->exec();
}

void MainWindow::updateVoltageMrad(float voltage) {
    if (!Manager->haveStructure() || !Manager->haveResolution())
        return;

    // update the voltage in teh manager
    Manager->getMicroscopeParams()->Voltage = voltage;

    ui->tSim->updateResolutionInfo(Manager->getRealScale(), Manager->getInverseScale(), Manager->getInverseMaxAngle());
}



void MainWindow::on_actionTheme_triggered() {
#ifdef _WIN32
    // open out theme selector dialog
    ThemeDialog* myDialog = new ThemeDialog(this);
    myDialog->exec();
#endif
}

void MainWindow::on_actionImport_default_triggered(bool preserve_ui) {
    // use preserve ui because the action defaults to false (even with the default arg set to true)
    QSettings settings;

    if (!settings.contains("defaultParameters"))
        settings.setValue("defaultParameters", "default");

    std::string param_name = settings.value("defaultParameters").toString().toStdString();

    // try loading default settings from the config location
    QString config_location = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if(config_location.endsWith("/"))
        config_location.chop(1);

    try {
        nlohmann::json j = fileio::OpenSettingsJson(config_location.toStdString() + "/microscopes/" + param_name + ".json");
        *Manager = JSONUtils::JsonToManager(j);
    } catch (const std::runtime_error& e) {
        CLOG(ERROR, "gui") << "Problem importing json settings: " << e.what();
        // don't worry, we'll just use the default settings
        Manager = std::make_shared<SimulationManager>();
    }

    if (!preserve_ui)
        updateGuiFromManager();
}

void MainWindow::on_actionExport_default_triggered() {
    QSettings settings;

    if (!settings.contains("defaultParameters"))
        settings.setValue("defaultParameters", "default");

    std::string param_name = settings.value("defaultParameters").toString().toStdString();

    // try loading default settings from the config location
    QString config_location = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if(config_location.endsWith("/"))
        config_location.chop(1);

    std::string dirName = config_location.toStdString() + "/microscopes/";
    std::string fileName = dirName + param_name + ".json";

    QFileInfo temp_file(QString::fromStdString(fileName));

    updateManagerFromGui();

    QDir dir(QString::fromStdString(dirName));
    if (!dir.exists())
        dir.mkpath(".");

    nlohmann::json j = JSONUtils::FullManagerToJson(*Manager);
    fileio::SaveSettingsJson(fileName, j);
}

void MainWindow::on_actionShow_default_triggered() {
    QSettings settings;

    if (!settings.contains("defaultParameters"))
        settings.setValue("defaultParameters", "default");

    std::string param_name = settings.value("defaultParameters").toString().toStdString();

    // try loading default settings from the config location
    QString config_location = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if(config_location.endsWith("/"))
        config_location.chop(1);

    std::string dirName = config_location.toStdString() + "/microscopes/"; // don't add the file, so we open in the file browser

    QDir dir(QString::fromStdString(dirName));
    if (!dir.exists())
        dir.mkpath(".");

    GuiUtils::openInDefault(QString::fromStdString(dirName));
}
