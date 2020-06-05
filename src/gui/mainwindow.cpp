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
#include <cif/supercell.h>

#include <variant>
#include <frames/inelasticframe.h>
#include <controls/tabpanel.h>

MainWindow::MainWindow(QWidget *parent) :
    BorderlessWindow(parent),
    ui(new Ui::MainWindow)
{
    // opencl test has been moved to main.cpp

    // register types for our image returns!!
    qRegisterMetaType< std::map<std::string, Image<float>> >( "std::map<std::string, Image<float>>" );
    qRegisterMetaType< std::map<std::string, Image<double>> >( "std::map<std::string, Image<double>>" );
    qRegisterMetaType< SimulationManager >( "SimulationManager" );

    QSettings settings;
    if (!settings.contains("dialog/currentPath"))
        settings.setValue("dialog/currentPath", QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    if (!settings.contains("dialog/currentSavePath"))
        settings.setValue("dialog/currentSavePath", QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

    Manager = std::make_shared<SimulationManager>();

    std::string exe_path = QGuiApplication::applicationDirPath().toStdString();

    // try loading default settings from the config location
    on_actionImport_default_triggered(true);

    loadSavedOpenClSettings();

    bool live_stem_set = settings.value("live stem").toBool();
    Manager->setLiveStemEnabled(live_stem_set);

    ui->setupUi(this);

    auto pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));
    ui->edtIterations->setValidator(pIntValidator);

    setWindowTitle("clTEM");

    dynamic_cast<tabPanel*>(ui->twSim)->setPreserveHeightEnabled(true);

    // this just makes it look nice without fannying about with widgets and all that
    int w = width();
    resize(w, w*0.6);

    auto* Img = new ImageTab(ui->twReal, "Image", TabType::CTEM);
    auto* EwAmp = new ImageTab(ui->twReal, "EW", TabType::CTEM, true);
    auto* Diff = new ImageTab(ui->twReal, "Diffraction", TabType::DIFF);

    ui->twReal->addTab(Img, QString::fromStdString(Img->getTabName()));
    ui->twReal->addTab(EwAmp, QString::fromStdString(EwAmp->getTabName()));
    ui->twReal->addTab(Diff, QString::fromStdString(Diff->getTabName()));

    StatusBar = new StatusLayout();

    ui->statusBar->addWidget(StatusBar, 100);

    // this is required so the frame and then dialog can access the current aberrations at any time
    // could be avoided but I've used Qt designer with the .ui files and so on

    ui->tSim->assignMainWindow(this);

    ui->tMicroscope->assignMainWindow(this);
    ui->tAberr->assignMainWindow(this);
    ui->tInelastic->assignMainWindow(this);
    ui->tIncoherence->assignMainWindow(this);

    ui->tStem->assignMainWindow(this);
    ui->tCbed->assignMainWindow(this);

    connect(ui->tSim, &SimulationFrame::resolutionSet, this, &MainWindow::resolution_changed);

    connect(ui->btnStartSim, &QPushButton::clicked, this, &MainWindow::on_actionSimulate_EW_triggered);
    connect(ui->btnCancelSim, &QPushButton::clicked, this, &MainWindow::cancel_simulation);

    connect(ui->tTem, &TemFrame::setCtemCrop, this, &MainWindow::set_ctem_crop);
    connect(ui->tTem, &TemFrame::setCtemImage, this, &MainWindow::ctemImageToggled);

    connect(this, &MainWindow::sliceProgressUpdated, this, &MainWindow::sliceProgressChanged);
    connect(this, &MainWindow::totalProgressUpdated, this, &MainWindow::totalProgressChanged);
    connect(this, &MainWindow::imagesReturned, this, &MainWindow::imagesChanged);

    connect(ui->edtIterations, &QLineEdit::textChanged, this, &MainWindow::checkEditZero);

    connect(ui->tInelastic, &InelasticFrame::iterationsCheckedChanged, this, &MainWindow::iterationsToggled);
    connect(ui->tIncoherence, &IncoherenceFrame::iterationsCheckedChanged, this, &MainWindow::iterationsToggled);

    int n = ui->twReal->count();
    for (int j = 0; j < n; ++j) {
        auto *tab = (ImageTab *) ui->twReal->widget(j);
        connect(tab, &ImageTab::saveDataActivated, this, &MainWindow::saveTiff);
        connect(tab, &ImageTab::saveImageActivated, this, &MainWindow::saveBmp);
    }

    try {
        loadExternalSources();
    } catch (std::exception& e) {
        QMessageBox msgBox(this);
        msgBox.setText("Error:");
        msgBox.setInformativeText(e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setMinimumSize(160, 125);
        msgBox.exec();
    }

    iterationsToggled();

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
            bool try_fix = false;

            CIF::CIFReader cif;

            try {
                cif = CIF::CIFReader(fileName.toStdString(), false);
            } catch (const std::exception &e) {
                CLOG(ERROR, "gui") << "Could not open file: " << e.what() << ".";
                QMessageBox msgBox(this);
                msgBox.setText("Error:");
                msgBox.setInformativeText(e.what());
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setMinimumSize(160, 125);
                msgBox.exec();

                auto reply = QMessageBox::question(this, "Error:", "Would you like me to try and fix this cif?", QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes)
                    cif = CIF::CIFReader(fileName.toStdString(), true);
                else
                    return;
            }

            auto info = std::make_shared<CIF::SuperCellInfo>();

            auto myDialog = new CifCreatorDialog(this, cif, info);
            auto result = myDialog->exec();

            if(result == QDialog::Accepted)
                Manager->setStructure(cif, *info);
            else
                return;
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

    ui->tCbed->updateTextBoxes();

    ui->tStem->updateScaleLabels();

    StatusBar->setFileLabel(fileName);
}

void MainWindow::on_actionOpenCL_triggered()
{
    // TODO: later will want to pass the full tuple (performance factors and all)
    OpenClDialog *myDialog = new OpenClDialog(this, Devices);

    myDialog->exec();
}

void MainWindow::resolution_changed(int resolution)
{
    // set resolution in manager
    Manager->setResolution(resolution);

    updateScales();
}

void MainWindow::updateScales()
{
    if (!Manager->simulationCell()->crystalStructure() || !Manager->resolutionValid())
        return;

    try {
        ui->tSim->updateResolutionInfo(Manager->realScale(), Manager->inverseScale(),
                                       Manager->inverseMaxAngle());
    } catch (...) {}
    ui->tSim->updateStructureInfo(Manager->simRanges());
}

void MainWindow::updateRanges()
{
    if (!Manager->simulationCell()->crystalStructure())
        return;

    ui->tSim->updateStructureInfo(Manager->simRanges());;
}

void MainWindow::setDetectors()
{
    //Manager->setDetectors(d);

    // this adds any detectors that are new
    for (auto d : Manager->stemDetectors())
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
            for (auto d : Manager->stemDetectors())
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

    updateModeTextBoxes();
    updateScales();
}

void MainWindow::updateSlicesProgress(double prog)
{
    QMutexLocker locker(&Progress_Mutex);
    emit sliceProgressUpdated(prog);
}

void MainWindow::updateTotalProgress(double prog)
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

    StatusBar->setSliceProgress(0.0);
    StatusBar->setTotalProgress(0.0);

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

    // TODO: move to the manager class (then we can easily call it from the command line too)
    // TODO: operate on an exception basis...
    // Check our plasmon configuration is viable
    if (Manager->incoherenceEffects()->plasmons()->enabled()) {
        int parts = Manager->totalParts();
        Manager->incoherenceEffects()->plasmons()->initDepthVectors(parts);
        auto z_lims = Manager->simulationCell()->crystalStructure()->limitsZ();
        double thk = z_lims[1] - z_lims[0];

        bool valid = false;
        for (int i = 0; i < parts; ++i) {
            valid = Manager->incoherenceEffects()->plasmons()->generateScatteringDepths(i, thk);

            if (!valid) {
                QMessageBox msgBox(this);
                msgBox.setText("Error:");
                //            msgBox.setInformativeText(e.what());
                msgBox.setInformativeText("Could not generate valid plasmon configuration.");
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setMinimumSize(160, 125);
                msgBox.exec();
                setUiActive(true);
                return;
            }

        }
    }

    std::vector<std::shared_ptr<SimulationManager>> man_list; //why is this a vector?

    auto sliceRep = std::bind(&MainWindow::updateSlicesProgress, this, std::placeholders::_1);
    Manager->setProgressSliceReporterFunc(sliceRep);

    auto totalRep = std::bind(&MainWindow::updateTotalProgress, this, std::placeholders::_1);
    Manager->setProgressTotalReporterFunc(totalRep);

    auto imageRet = std::bind(&MainWindow::updateImages, this, std::placeholders::_1);
    Manager->setImageReturnFunc(imageRet);

    bool use_double_precision = Manager->doublePrecisionEnabled();

    auto temp = std::make_shared<SimulationManager>(*Manager);

    man_list.push_back(temp);

    std::vector<clDevice> &d = Devices;

    SimThread = std::make_shared<SimulationThread>(man_list, d, use_double_precision);

    SimThread->start();
}

void MainWindow::sliceProgressChanged(double prog)
{
    StatusBar->setSliceProgress(prog);
}

void MainWindow::totalProgressChanged(double prog)
{
    StatusBar->setTotalProgress(prog);
}

void MainWindow::imagesChanged(SimulationManager sm)
{

    auto ims = sm.images();

    if (ims.empty()) {
        simulationFailed();
    }

    nlohmann::json original_settings = JSONUtils::BasicManagerToJson(sm, false, true);
    original_settings["filename"] = sm.simulationCell()->crystalStructure()->fileName();

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
                    auto settings = original_settings;
                    settings["microscope"].erase("aberrations");
                    settings["microscope"].erase("alpha");
                    settings["microscope"].erase("delta");

                    auto pd = im.getPadding(); // t l b r
                    Image<std::complex<double>> comp_im(im.getWidth(), im.getHeight(), im.getDepth(), pd[0], pd[1], pd[2], pd[3]);

                    for (int jj = 0; jj < im.getDepth(); ++jj) {
                        // convert our float data to complex
                        std::vector<std::complex<double>> comp_data(im.getSliceSize());
                        for (int ii = 0; ii < comp_data.size(); ++ii)
                            comp_data[ii] = std::complex<double>(im.getSliceRef(jj)[2 * ii], im.getSliceRef(jj)[2 * ii + 1]);

                        comp_im.getSliceRef(jj) = comp_data;
                    }

                    double lx = sm.paddedSimLimitsX(0)[0];
                    double ly = sm.paddedSimLimitsY(0)[0];
                    double sc = sm.realScale();
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
                    auto settings = original_settings;
                    double lx = sm.paddedSimLimitsX(0)[0];
                    double ly = sm.paddedSimLimitsY(0)[0];
                    double sc = sm.realScale();
                    tab->setPlotWithData(im, "Å", sc, sc, lx, ly, settings);
                }
            }
        }
        else if (name == "Diff")
        {
            int n = ui->twReal->count();
            for (int j = 0; j < n; ++j)
            {
                ImageTab *tab = (ImageTab *) ui->twReal->widget(j);
                if (tab->getTabName() == "Diffraction") {
                    auto settings = original_settings;
                    settings["microscope"].erase("aberrations");
                    settings["microscope"].erase("alpha");
                    settings["microscope"].erase("delta");
                    double sc;
                    QString unit;
                    if (sm.mode() == SimulationMode::CBED) {
                        sc = sm.inverseScaleAngle();
                        unit = "mrad";
                    } else {
                        sc = sm.inverseScale();
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
                    auto settings = original_settings;
                    for (auto d : sm.stemDetectors())
                        if (d.name == name)
                            settings["stem"]["detectors"][d.name] = JSONUtils::stemDetectorToJson(d);
                    settings["microscope"].erase("alpha");
                    settings["microscope"].erase("delta");

                    double lx = sm.stemArea()->getRawLimitsX()[0];
                    double ly = sm.stemArea()->getRawLimitsY()[0];
                    double scx = sm.stemArea()->getScaleX();
                    double scy = sm.stemArea()->getScaleY();

                    tab->setPlotWithData(im, "Å", scx, scy, lx, ly, settings);
                }
            }
        }
    }

    if (sm.allPartsCompleted())
        simulationComplete();
}

void MainWindow::setUiActive(bool active)
{
    //disable things the user shouldn't be able to access whilst a simulation is running
    ui->btnStartSim->setEnabled(active);
}

void MainWindow::loadSavedOpenClSettings()
{
    // SET THE SAVED OPENCL DEVICES (IF IT EXISTS STILL)
    // seemingly can't compare device id, so compare platform/device numbers and name
    // these might change with hardware changes, but not enough to be annoying (I think)
    QSettings settings;
    settings.beginGroup("opencl");

    bool mad = settings.value("opts/mad").toBool();
    bool no_signed = settings.value("opts/no_signed").toBool();
    bool unsafe_maths = settings.value("opts/unsafe_maths").toBool();
    bool finite_maths = settings.value("opts/finite_maths").toBool();
    bool native_maths = settings.value("opts/native_maths").toBool();

    KernelSource::setOptions(mad, no_signed, unsafe_maths, finite_maths, native_maths);

    QStringList devs = settings.childGroups();
    std::vector<clDevice> dev_list;
    auto present_devs = OpenCL::GetDeviceList();
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

    sliceProgressChanged(0.0);
    totalProgressChanged(0.0);
}

void MainWindow::loadExternalSources()
{
    // Populate the kernels from files...
    Kernels::atom_sort_f = Utils_Qt::kernelToChar("atom_sort_f.cl");
    Kernels::band_limit_f = Utils_Qt::kernelToChar("band_limit_f.cl");
    Kernels::band_pass_f = Utils_Qt::kernelToChar("band_pass_f.cl");
    Kernels::ccd_dqe_f = Utils_Qt::kernelToChar("ccd_dqe_f.cl");
    Kernels::ccd_ntf_f = Utils_Qt::kernelToChar("ccd_ntf_f.cl");
    Kernels::complex_multiply_f = Utils_Qt::kernelToChar("complex_multiply_f.cl");
    Kernels::ctem_image_f = Utils_Qt::kernelToChar("ctem_image_f.cl");
    Kernels::fft_shift_f = Utils_Qt::kernelToChar("fft_shift_f.cl");
    Kernels::init_plane_wave_f = Utils_Qt::kernelToChar("init_plane_wave_f.cl");
    Kernels::init_probe_wave_f = Utils_Qt::kernelToChar("init_probe_wave_f.cl");
    Kernels::transmission_potentials_full_3d_f = Utils_Qt::kernelToChar("transmission_potentials_full_3d_f.cl");
    Kernels::transmission_potentials_projected_f = Utils_Qt::kernelToChar("transmission_potentials_projected_f.cl");
    Kernels::propagator_f = Utils_Qt::kernelToChar("propagator_f.cl");
    Kernels::sqabs_f = Utils_Qt::kernelToChar("sqabs_f.cl");
    Kernels::sum_reduction_f = Utils_Qt::kernelToChar("sum_reduction_f.cl");
    Kernels::bilinear_translate_f = Utils_Qt::kernelToChar("bilinear_translate_f.cl");
    Kernels::complex_to_real_f = Utils_Qt::kernelToChar("complex_to_real_f.cl");

    Kernels::atom_sort_d = Utils_Qt::kernelToChar("atom_sort_d.cl");
    Kernels::band_limit_d = Utils_Qt::kernelToChar("band_limit_d.cl");
    Kernels::band_pass_d = Utils_Qt::kernelToChar("band_pass_d.cl");
    Kernels::ccd_dqe_d = Utils_Qt::kernelToChar("ccd_dqe_d.cl");
    Kernels::ccd_ntf_d = Utils_Qt::kernelToChar("ccd_ntf_d.cl");
    Kernels::complex_multiply_d = Utils_Qt::kernelToChar("complex_multiply_d.cl");
    Kernels::ctem_image_d = Utils_Qt::kernelToChar("ctem_image_d.cl");
    Kernels::fft_shift_d = Utils_Qt::kernelToChar("fft_shift_d.cl");
    Kernels::init_plane_wave_d = Utils_Qt::kernelToChar("init_plane_wave_d.cl");
    Kernels::init_probe_wave_d = Utils_Qt::kernelToChar("init_probe_wave_d.cl");
    Kernels::transmission_potentials_full_3d_d = Utils_Qt::kernelToChar("transmission_potentials_full_3d_d.cl");
    Kernels::transmission_potentials_projected_d = Utils_Qt::kernelToChar("transmission_potentials_projected_d.cl");
    Kernels::propagator_d = Utils_Qt::kernelToChar("propagator_d.cl");
    Kernels::sqabs_d = Utils_Qt::kernelToChar("sqabs_d.cl");
    Kernels::sum_reduction_d = Utils_Qt::kernelToChar("sum_reduction_d.cl");
    Kernels::bilinear_translate_d = Utils_Qt::kernelToChar("bilinear_translate_d.cl");
    Kernels::complex_to_real_d = Utils_Qt::kernelToChar("complex_to_real_d.cl");

    // load parameters
    // get all the files in the parameters folder
    auto params_path = QGuiApplication::applicationDirPath() + "/params/";
    QDir params_dir(params_path);
    QStringList params_filt;
    params_filt << "*.dat";
    QStringList params_files = params_dir.entryList(params_filt);

    if (params_files.empty())
        throw std::runtime_error("Need at least one valid parameters file");

    for (int k = 0; k < params_files.size(); ++k)
        Utils_Qt::readParamsFile(params_files[k].toStdString());


    // load DQE, NQE for the CTEM simulation

    auto ccd_path = QGuiApplication::applicationDirPath() + "/ccds/";
    QDir ccd_dir(ccd_path);
    QStringList ccd_filt;
    ccd_filt << "*.dat";
    QStringList ccd_files = ccd_dir.entryList(ccd_filt);

    std::vector<double> dqe, ntf;
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

void MainWindow::updateAberrationBoxes() {
    ui->tAberr->updateTextBoxes();
    ui->tMicroscope->updateTextBoxes();
    ui->tIncoherence->updateTemTextBoxes();
}

void MainWindow::updateAberrationManager() {
    ui->tAberr->updateAberrations();
    ui->tIncoherence->updateManager();
    ui->tMicroscope->updateManagerFromGui();
}

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

void MainWindow::saveTiff(bool full_stack) {
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

    nlohmann::json j_settings = origin->getSettings(); // get settings
    fileio::SaveSettingsJson(fo+".json", j_settings); // save settings

    if (full_stack) {
        // calculate the slice count of this output
        auto si = JSONUtils::readJsonEntry<unsigned int>(j_settings, "intermediate output", "slice interval");
        auto sc = JSONUtils::readJsonEntry<unsigned int>(j_settings, "slice count");
        if (si < 1)
            throw std::runtime_error("Saving stack with < 0 slice step.");

        int out_string_len = Utils::numToString(sc-1).size();

        for (int i = 0; i < origin->getPlot()->getSliceCount(); ++i) {
            origin->getPlot()->getData(data, sx, sy, i); // get data
            // get the name to use for the output
            //  remember we don't start getting slices from the first slice
            unsigned int slice_id = (i+1)*si-1;
            if (slice_id >= sc)
                slice_id = sc-1;
            std::string temp = Utils::uintToString(slice_id, out_string_len);

            fileio::SaveTiff<float>(fo+temp+".tif", data, sx, sy); // save data
        }

    } else {
        origin->getPlot()->getCurrentData(data, sx, sy); // get data
        fileio::SaveTiff<float>(fo+".tif", data, sx, sy); // save data
    }
}

void MainWindow::saveBmp(bool full_stack) {
    auto origin = dynamic_cast<ImageTab*>(sender());

    // do the dialog stuff
    QSettings settings;
    QString filepath = QFileDialog::getSaveFileName(this, "Save image", settings.value("dialog/currentSavePath").toString(), "Bitmap (*.bmp)");

    if (filepath.isEmpty())
        return;

    QFileInfo temp(filepath);
    settings.setValue("dialog/currentSavePath", temp.path());
    std::string fo = filepath.toStdString();

    // I feel that there should be a better way for this...
    // get the filepath without the extension (if it is there)
    if (fo.substr(fo.length() - 4) == ".bmp")
        fo = fo.substr(0, fo.length() - 4);

    // set up where we will get our data
    int sx, sy;
    std::vector<float> data;

    nlohmann::json j_settings = origin->getSettings(); // get settings
    fileio::SaveSettingsJson(fo + ".json", j_settings); // save settings

    if (full_stack) {
        // calculate the slice count of this output
        auto si = JSONUtils::readJsonEntry<unsigned int>(j_settings, "intermediate output", "slice interval");
        auto sc = JSONUtils::readJsonEntry<unsigned int>(j_settings, "slice count");
        if (si < 1)
            throw std::runtime_error("Saving stack with < 0 slice step.");

        int out_string_len = Utils::numToString(sc-1).size();

        for (int i = 0; i < origin->getPlot()->getSliceCount(); ++i) {
            origin->getPlot()->getData(data, sx, sy, i); // get data

            // get the name to use for the output
            //  remember we don't start getting slices from the first slice
            unsigned int slice_id = (i+1)*si-1;
            if (slice_id >= sc)
                slice_id = sc-1;
            std::string temp = Utils::uintToString(slice_id, out_string_len);

            fileio::SaveBmp(fo + temp + ".bmp", data, sx, sy); // save data
        }

    } else {
        origin->getPlot()->getCurrentData(data, sx, sy); // get data
        fileio::SaveBmp(fo + ".bmp", data, sx, sy); // save data
    }
}

void MainWindow::on_actionGeneral_triggered() {
    GlobalSettingsDialog *myDialog = new GlobalSettingsDialog(this, Manager);

    // this is in case of padding changes or stem parallel pixels
    connect(myDialog, &GlobalSettingsDialog::appliedSignal, this, &MainWindow::updateScales);

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
    auto structure = Manager->simulationCell()->crystalStructure();
    *Manager = m;
    Manager->setStructure(structure);

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
    // update incoherent/inelastic iterations
    unsigned int its = ui->edtIterations->text().toUInt();
    Manager->incoherenceEffects()->setIterations(its);

    ui->tMicroscope->updateManagerFromGui();

    ui->tInelastic->updateManager();

    ui->tIncoherence->updateManager();

    // update aberrations from the main tab
    // aberrations in the dialog are updated when you click apply
    ui->tAberr->updateAberrations();

    // load variables for potential TEM stuff
    Manager->setCcdBinning(ui->tTem->getBinning());
    Manager->setCtemImageEnabled(ui->tTem->getSimImage());
    Manager->setCcdName(ui->tTem->getCcd());
    Manager->setCcdDose(ui->tTem->getDose());
}

void MainWindow::updateGuiFromManager() {
    // update the iterations textbox
    ui->edtIterations->setText(QString::number(Manager->incoherenceEffects()->storedIterations()));

    // set aberrations frame
    // set microscope frame
    // set inelastic frame
    updateAberrationBoxes();

    // set CBED stuff (position/TDS)
    ui->tCbed->updateTextBoxes();

    // set STEM TDS
    ui->tStem->updateScaleLabels();

    // set inelastic frame parameters
    ui->tInelastic->updateGui();

    ui->tIncoherence->updateTextBoxes();

    // set CTEM CCD stuff
    ui->tTem->update_ccd_boxes(Manager);

    // update the detector tabs
    setDetectors();

    ui->tTem->setSimImageCheck( Manager->ctemImageEnabled() );

    ui->twMode->setCurrentIndex( (int) Manager->mode()-1 ); // -1 due to the 0 element being a null value

    // set resolution last, this should update the structure area stuff if it needs to be
    ui->tSim->setResolution( Manager->resolution() );
}

void MainWindow::on_actionSet_area_triggered()
{
    updateManagerFromGui();

    SimAreaDialog* myDialog = new SimAreaDialog(this, Manager);

    // I could simplify this to  use the 'appliedSignal' but there is a lot going on.
    // I also need to think about the behaviour of clicking apply whilst data has changed on the various tabs
    // i.e. do I update all of them, or just the one being shown at the moment
    connect(myDialog->getFrame(), &AreaLayoutFrame::resolutionChanged, ui->tSim, &SimulationFrame::setResolutionText);
    connect(myDialog->getFrame(), &AreaLayoutFrame::modeChanged, this, &MainWindow::set_active_mode);
    connect(myDialog->getFrame(), &AreaLayoutFrame::updateMainCbed, getCbedFrame(), &CbedFrame::updateTextBoxes);
    connect(myDialog->getFrame(), &AreaLayoutFrame::updateMainStem, getStemFrame(), &StemFrame::updateScaleLabels);
    connect(myDialog->getFrame(), &AreaLayoutFrame::areaChanged, this, &MainWindow::updateScales);

    myDialog->exec();
}

void MainWindow::on_actionAberrations_triggered()
{
    // here we update the current aberrations from the text boxes here so the dialog can show the same
    updateAberrationManager();

    auto* myDialog = new AberrationsDialog(this, Manager);

    connect(myDialog, &AberrationsDialog::appliedSignal, this, &MainWindow::updateAberrationBoxes);

    myDialog->exec();
}

void MainWindow::on_actionThermal_scattering_triggered() {
    auto* myDialog = new ThermalScatteringDialog(this, Manager);
    connect(myDialog, &ThermalScatteringDialog::appliedSignal, ui->tInelastic, &InelasticFrame::updatePhononsGui);
    myDialog->exec();
}

void MainWindow::on_actionPlasmons_triggered() {
    auto* myDialog = new PlasmonDialog(this, Manager);
    connect(myDialog, &PlasmonDialog::appliedSignal, ui->tInelastic, &InelasticFrame::updatePlasmonsGui);
    myDialog->exec();
}

void MainWindow::updateVoltageMrad(double voltage) {
    if (!Manager->simulationCell()->crystalStructure() || !Manager->resolutionValid())
        return;

    // update the voltage in teh manager
    Manager->microscopeParams()->Voltage = voltage;

    try {
    ui->tSim->updateResolutionInfo(Manager->realScale(), Manager->inverseScale(), Manager->inverseMaxAngle());
    } catch (...) {}
}



void MainWindow::on_actionTheme_triggered() {
    // open out theme selector dialog
    ThemeDialog* myDialog = new ThemeDialog(this, Manager);
    myDialog->exec();
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

    auto structure = Manager->simulationCell()->crystalStructure();

    try {
        nlohmann::json j = fileio::OpenSettingsJson(config_location.toStdString() + "/microscopes/" + param_name + ".json");
        *Manager = JSONUtils::JsonToManager(j);
    } catch (const std::runtime_error& e) {
        CLOG(ERROR, "gui") << "Problem importing json settings: " << e.what();
        // don't worry, we'll just use the default settings
        Manager = std::make_shared<SimulationManager>();
    }

    Manager->setStructure(structure);

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

void MainWindow::updateModeTextBoxes() {
    auto md = Manager->mode();
    bool tem_image = Manager->ctemImageEnabled();

    ui->tMicroscope->setModeStyles(md, tem_image);
    ui->tAberr->setModeStyles(md, tem_image);
    ui->tIncoherence->setModeStyles(md, tem_image);
}

void MainWindow::iterationsToggled() {
    // update all the iterations frames



    ui->tInelastic->updateManager();
    ui->tIncoherence->updateManager();

    bool use = Manager->incoherenceEffects()->enabled(Manager->mode());

    if (use) {
        ui->edtIterations->setUnits("");
    } else {
        ui->edtIterations->setUnits("(N/A)");
    }

    ui->edtIterations->update();
}

bool MainWindow::event(QEvent *event) {
    // this might get spammed a bit, not sure if it is supposed to
    if (event->type() == QEvent::PaletteChange)
    {
        auto md = Manager->mode();
        auto im = Manager->ctemImageEnabled();

//        iterationsToggled();
        ui->tMicroscope->setModeStyles(md, im);
        ui->tAberr->setModeStyles(md, im);
        ui->tIncoherence->setModeStyles(md, im);
    }

    // very important or no other events will get through
    return BorderlessWindow::event(event);
}
