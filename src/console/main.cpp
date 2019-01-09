#include <iostream>
#include <vector>
#include <list>
#include <clwrapper/clstatic.h>
#include <simulationmanager.h>
#include <utilities/json.hpp>
#include <utilities/fileio.h>
#include <utilities/jsonutils.h>

#include "getopt.h"
#include "parseopencl.h"

#include <boost/filesystem.hpp>
#include <simulationrunner.h>
#include <structure/structureparameters.h>
#include <kernels.h>
#ifdef _WIN32

#include "windows.h"
#include <libgen.h>

#else

#include <libgen.h>
#include <zconf.h>

#endif


namespace fs = boost::filesystem;

static std::string out_path;

static std::mutex out_mtx;

static int total_pcnt;
static int slice_pcnt;

void printHelp()
{
    std::cout << "usage: cltem_cmd xyz_file [options]\n"
                 "  options:\n"
                 "    -h : (--help) print this help message and exit\n"
                 "    -v : (--version) print the clTEM command line version number and exit\n"
                 "    -l : (--list) print the available OpenCL devices and exit\n"
                 "    -o : (--output) REQUIRED set the output directory for the simulation results\n"
                 "    -c : (--config) REQUIRED set the .json config file for the simulation\n"
                 "    -d : (--device) REQUIRED set the OpenCL device(s). Accepts format:\n"
                 "             default : uses the default OpenCL device(s)\n"
                 "             all     : uses all available OpenCL device(s)\n"
                 "             gpus    : use all available gpus\n"
                 "             cpus    : use all available cpus\n"
                 "             gpu     : use the first gpu available\n"
                 "             cpu     : use the first cpu available\n"
                 "             #:#     : comma separated list in for format platform:device (ids)\n"
                 "    --verbose : show full output" << std::endl;
}

void printVersion()
{
    std::cout << "clTEM command line interface v0.2a" << std::endl;
}

void listDevices()
{
    auto devices = OpenCL::GetDeviceList(Device::DeviceType::All);

    std::cout << "OpenCL devices available" << std::endl;

    int prev_plat = -1;
    for (clDevice& d : devices)
    {
        if (prev_plat != d.GetPlatformNumber())
            std::cout << "Platform: " << d.GetPlatformNumber() << ", " << d.GetPlatformName() << std::endl;
        std::cout << "\tDevice: " << d.GetDeviceNumber() << ", " << d.GetDeviceName() << std::endl;
    }
}

void reportSliceProgress(float frac)
{
    std::lock_guard<std::mutex> lock(out_mtx);
    // currently only updates once hte simulation has finished...
    int slice_pcnt = (int) (frac*100);


    std::cout << "Slice progress: " << slice_pcnt << "%, total progress: " << total_pcnt << "%          \r" << std::flush;
    if (total_pcnt >= 100.0f)
        std::cout << std::endl;
}

void reportTotalProgress(float frac)
{
    std::lock_guard<std::mutex> lock(out_mtx);
    total_pcnt = (int) (frac*100);


    std::cout << "Slice progress: " << slice_pcnt << "%, total progress: " << total_pcnt << "%          \r" << std::flush;
    if (total_pcnt >= 100.0f)
        std::cout << std::endl;
}

void imageReturned(std::map<std::string, Image<float>> ims, SimulationManager sm)
{
    nlohmann::json settings = JSONUtils::BasicManagerToJson(sm);
    settings["filename"] = sm.getStructure()->getFileName();

    // save the images....
    // we've been given a list of images, got to display them now....
    for (auto const& i : ims)
    {
        std::string name = i.first;
        auto im = i.second;
        // Currently assumes the positions of all the tabs

        if (name == "EW" || name == "Diff")
        {
            settings["microscope"].erase("aberrations");
            settings["microscope"].erase("alpha");
            settings["microscope"].erase("delta");
        }
        else
        {
            // add the specific detector info here!
            for (auto d : sm.getDetectors())
                if (d.name == name)
                    settings["stem"]["detectors"][d.name] = JSONUtils::stemDetectorToJson(d);
            settings["microscope"].erase("alpha");
            settings["microscope"].erase("delta");
        }


        if (name == "EW") { // save amplitude and phase
            std::vector<float> abs(im.data.size());
            std::vector<float> arg(im.data.size());

            for (int j = 0; j < im.data.size(); ++j) {
                abs[j] = std::abs(im.data[j]);
                arg[j] = std::arg(im.data[j]);
            }

            fileio::SaveTiff<float>(out_path + "/" + name + "_amplitude.tif", abs, im.width, im.height);
            fileio::SaveSettingsJson(out_path + "/" + name + "_amplitude.json", settings);

            fileio::SaveTiff<float>(out_path + "/" + name + "_phase.tif", arg, im.width, im.height);
            fileio::SaveSettingsJson(out_path + "/" + name + "_phase.json", settings);
        } else {
            fileio::SaveTiff<float>(out_path + "/" + name + ".tif", im.data, im.width, im.height);
            fileio::SaveSettingsJson(out_path + "/" + name + ".json", settings);
        }
    }
}

int main(int argc, char *argv[])
{
    int verbose_flag = 0;
    total_pcnt = 0;
    slice_pcnt = 0;
    int c;

    std::string output_dir = "";
    std::string input_struct = "";
    std::string input_params = "";

    std::string device_options = "";

    std::vector<std::string> non_option_args;

    while (true)
    {
        static struct option long_options[] =
                {
                        {"help",     no_argument,       nullptr,       'h'},
                        {"version",  no_argument,       nullptr,       'v'},
                        {"list_devices",  no_argument,       nullptr,       'l'},
                        {"output",   required_argument, nullptr,       'o'},
                        {"config",   required_argument, nullptr,       'c'},
                        {"device",   required_argument, nullptr,       'd'},
                        {"verbose",  no_argument,       &verbose_flag, 1},
                        {nullptr, 0, nullptr, 0}
                };
        // getopt_long stores the option index here.
        int option_index = 0;
        c = getopt_long (argc, argv, "hvlo:c:d:V", long_options, &option_index);

        // Detect the end of the options.
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                break;
            case 'h':
                printHelp();
                return 0;
            case 'v':
                printVersion();
                return 0;
            case 'l':
                listDevices();
                return 0;
            case 'o':
                output_dir = optarg;
                break;
            case 'c':
                input_params = optarg;
                break;
            case 'd':
                device_options = optarg;
                break;
            case '?':
                // getopt_long already printed an error message.
                break;

            default:
                return 1;
        }
    }

    if (verbose_flag) {
        std::cout << "Verbose flag is set" << std::endl << std::endl;
    }

    // get the non option args
    while (optind < argc)
        non_option_args.emplace_back(argv[optind++]);


    // check the options we need have at least been entered
    bool valid_flags = true;

    if (non_option_args.empty()) {
        std::cerr << "Require non-option argument as structure (.xyz) file" << std::endl;
        valid_flags = false;
    }

    if (output_dir.empty()) {
        std::cerr << "Require output directory argument (-o, --output)" << std::endl;
        valid_flags = false;
    }

    if (device_options.empty()) {
        std::cerr << "Require OpenCL device(s) to be selected (-d, --device)" << std::endl;
        valid_flags = false;
    }

    if (input_params.empty()) {
        std::cerr << "Require a configuration file argument (-c, --config)" << std::endl;
        valid_flags = false;
    }

    if (non_option_args.size() > 1) {
        std::cerr << "Only expecting one non-option argument. Instead got:" << std::endl;
        for (std::string& s : non_option_args)
            std::cerr << s << std::endl;
        valid_flags = false;
    }

    if (!valid_flags)
        return 1;

    // this is where the input is actually set
    input_struct = non_option_args[0];

    std::cout << "Getting OpenCL devices:" << std::endl;

    std::vector<clDevice> device_list = getDevices(device_options);

    if (device_list.size() < 1) {
        std::cout << "No valid OpenCL device selected. Exiting..." << std::endl;
        return 1;
    }

    std::cout << std::endl;
    // now have all our files/folders
    // need to check that they are all valid!

    std::vector<std::shared_ptr<SimulationManager>> man_list;

    // read the config file in

    nlohmann::json j;

    std::cout << "Config file: " << input_params << std::endl;
    try {
        j = fileio::OpenSettingsJson(input_params);
    } catch (...) {
        std::cout << "Error opening config file. Exiting..." << std::endl;
        return 1;
    }

    // make our manager...
    auto man_ptr = std::make_shared<SimulationManager>(JSONUtils::JsonToManager(j));

    // try to open the structure file...
    std::cout << "Structure file: " << input_struct << std::endl;
    try {
        man_ptr->setStructure(input_struct);
    } catch (...) {
        std::cout << "Error opening structure file. Exiting..." << std::endl;
        return 1;
    }

    auto sliceRep = reportSliceProgress;
    man_ptr->setProgressSliceReporterFunc(sliceRep);

    auto totalRep = reportTotalProgress;
    man_ptr->setProgressTotalReporterFunc(totalRep);

    auto imageRet = imageReturned;
    man_ptr->setImageReturnFunc(imageRet);

    std::cout << "Output directory: " << output_dir << std::endl;
    fs::path dir(output_dir);
    if (!fs::is_directory(dir)) {
        std::cout << "Directory does not exist. Attempting to create..." << std::endl;
        bool good = true;
        try {
            good = fs::create_directory(dir);
        } catch (fs::filesystem_error& e) {
            std::cout << "Error making directory. Exiting..." << std::endl;
            std::cout << e.what() << std::endl;
            return 1;
        }

        if (!good) {
            std::cout << "Error making directory. Exiting..." << std::endl;
            return 1;
        }
    }

    // load external sources

    std::string exe_path_string;

#ifdef _WIN32
    https://stackoverflow.com/a/13310600
    char exe_path[MAX_PATH];

    // When NULL is passed to GetModuleHandle, the handle of the exe itself is returned
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL) {
        // Use GetModuleFileName() with module handle to get the path
        GetModuleFileName(hModule, exe_path, MAX_PATH);

        auto exe_dir = dirname(exe_path);

        exe_path_string = std::string(exe_path);
    }
    else {
        std::cerr << "Cannot get executable path - Module handle is NULL" << std::endl ;
        return 1;
    }
#else
    // https://stackoverflow.com/questions/23943239/how-to-get-path-to-current-exe-file-on-linux
    char exe_path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", exe_path, PATH_MAX);
    const char *exe_dir;
    if (count != -1) {
        exe_dir = dirname(exe_path);
    }

    exe_path_string = std::string(exe_dir);
#endif
    // need to load potentials from external sources, then our manager is complete
    // TODO: do I want to bypass the static class? maybe it would help if we were loading a load of simulations to run..
    std::string params_path = exe_path_string + "/params";
    std::string p_name = JSONUtils::readJsonEntry<std::string>(j, "potentials");
    std::vector<float> params = Utils::paramsToVector(params_path, p_name+ ".dat");
    man_ptr->setStructureParameters(p_name, params);

    man_list.emplace_back(man_ptr);

    // open the kernels
    std::string kernel_path = exe_path_string + "/kernels";
    Kernels::atom_sort = Utils::resourceToChar(kernel_path, "atom_sort.cl");
    Kernels::floatSumReductionsource2 = Utils::resourceToChar(kernel_path, "sum_reduction.cl");
    Kernels::BandLimitSource = Utils::resourceToChar(kernel_path, "low_pass.cl");
    Kernels::fftShiftSource = Utils::resourceToChar(kernel_path, "post_fft_shift.cl");
    Kernels::opt2source = Utils::resourceToChar(kernel_path, "potential_full_3d.cl");
    Kernels::conv2source = Utils::resourceToChar(kernel_path, "potential_conventional.cl");
    Kernels::propsource = Utils::resourceToChar(kernel_path, "generate_propagator.cl");
    Kernels::multisource = Utils::resourceToChar(kernel_path, "complex_multiply.cl");
    Kernels::InitialiseWavefunctionSource = Utils::resourceToChar(kernel_path, "initialise_plane.cl");
    Kernels::imagingKernelSource = Utils::resourceToChar(kernel_path, "generate_tem_image.cl");
    Kernels::InitialiseSTEMWavefunctionSourceTest = Utils::resourceToChar(kernel_path, "initialise_probe.cl");
    Kernels::floatabsbandPassSource = Utils::resourceToChar(kernel_path, "band_pass.cl");
    Kernels::SqAbsSource = Utils::resourceToChar(kernel_path, "square_absolute.cl");
    Kernels::AbsSource = Utils::resourceToChar(kernel_path, "absolute.cl");
    Kernels::DqeSource = Utils::resourceToChar(kernel_path, "dqe.cl");
    Kernels::NtfSource = Utils::resourceToChar(kernel_path, "ntf.cl");

    std::string ccds_path = exe_path_string + "/ccds";

    std::string ccd_name = JSONUtils::readJsonEntry<std::string>(j, "ctem", "ccd", "name");

    if (ccd_name != "Perfect") {
        std::vector<float> dqe, ntf;
        std::string name;
        Utils::ccdToDqeNtf(ccds_path, ccd_name + ".dat", name, dqe, ntf);
        CCDParams::addCCD(name, dqe, ntf);
    }

    // global because I am lazy (or smart?)
    out_path = output_dir;

    auto simRunner = std::make_shared<SimulationRunner>(man_list, device_list);

    simRunner->runSimulations();

    return 0;
}
