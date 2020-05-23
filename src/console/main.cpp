#include <iostream>
#include <vector>
#include <list>
#include <clwrapper/clstatic.h>
#include <simulationmanager.h>
#include <utilities/json.hpp>
#include <utilities/fileio.h>
#include <utilities/jsonutils.h>
#include <utilities/simutils.h>

#include "getopt.h"
#include "parseopencl.h"

#include <boost/filesystem.hpp>

#include <threading/simulationrunner.h>
#include <structure/structureparameters.h>
#include <kernels.h>
#ifdef _WIN32

#include "windows.h"
#include <libgen.h>

#else

#include <libgen.h>
#include <zconf.h>

#endif

#include "utilities/logging.h"

namespace fs = boost::filesystem;

static std::string out_path;

static std::mutex out_mtx;

static int total_pcnt;
static int slice_pcnt;

void printHelp()
{
    std::cout << "usage: cltem_cmd structure_file [options]\n"
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
                 "    --debug : show full debug output\n"
                 "  .cif only options:\n"
                 "    -s : (--size) REQUIRED the size of the supercell (x,y,z values separated by commas)\n"
                 "    -z : (--zone) REQUIRED the zone axis to construct the structure along(u,v,w values separated by commas)\n"
                 "    -n : (--normal) the axis to place along the x-direction (u,v,w values separated by commas)\n"
                 "    -t : (--tilts) small tilts (degrees) to modify the zone axis (tilts around x,y,z axes separated by commas)"
                 "    --fix : attempt to fix some common errors in .cif files when parsing them\n" << std::endl;
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

/// Parse a string with 3 numbers separated by commas
template <typename T>
bool parseThreeCommaList(std::string lst, T& a, T& b, T& c)
{
    std::istringstream ss(lst);
    std::string part;
    T v;
    std::vector<T> vec;
    while(ss.good()) {
        getline( ss, part, ',' );
        std::istringstream ssp(part);
        ssp >> v;
        vec.emplace_back(v);
    }

    if (vec.size() != 3)
        return false;

    a = vec[0];
    b = vec[1];
    c = vec[2];

    return true;
}

void reportSliceProgress(double frac)
{
    std::lock_guard<std::mutex> lock(out_mtx);
    // currently only updates once hte simulation has finished...
    int slice_pcnt = (int) (frac*100);


    std::cout << "Slice progress: " << slice_pcnt << "%, total progress: " << total_pcnt << "%          \r" << std::flush;
    if (total_pcnt >= 100.0)
        std::cout << std::endl;
}

void reportTotalProgress(double frac)
{
    std::lock_guard<std::mutex> lock(out_mtx);
    total_pcnt = (int) (frac*100);


    std::cout << "Slice progress: " << slice_pcnt << "%, total progress: " << total_pcnt << "%          \r" << std::flush;
    if (total_pcnt >= 100.0)
        std::cout << std::endl;
}

void saveTiffOutput(std::string filename, Image<double> im, nlohmann::json j_settings) {

    if (filename.substr(filename.length() - 4) == ".tif")
        filename = filename.substr(0, filename.length() - 4);

    if (im.getDepth() > 1) {

        // calculate the slice count of this output
        auto si = JSONUtils::readJsonEntry<unsigned int>(j_settings, "intermediate output", "slice interval");
        auto sc = JSONUtils::readJsonEntry<unsigned int>(j_settings, "slice count");
        if (si < 1)
            throw std::runtime_error("Saving stack with < 0 slice step.");

        int out_string_len = Utils::numToString(sc-1).size();

        std::vector<double> data;

        for (int i = 0; i < im.getSliceSize(); ++i) {
            data = im.getSlice(i, false);

            // get the name to use for the output
            //  remember we don't start getting slices from the first slice
            unsigned int slice_id = (i+1)*si-1;
            if (slice_id >= sc)
                slice_id = sc-1;
            std::string temp = Utils::uintToString(slice_id, out_string_len);

            fileio::SaveTiff<float>(filename+"_"+temp+".tif", data, im.getWidth(), im.getHeight()); // save data
        }

    } else {
        fileio::SaveTiff<float>(filename+".tif", im.getSlice(0, false), im.getWidth(), im.getHeight()); // save data
    }
}

void imageReturned(SimulationManager sm)
{
    nlohmann::json settings = JSONUtils::BasicManagerToJson(sm);
    settings["filename"] = sm.simulationCell()->crystalStructure()->fileName();

#ifdef _WIN32
    std::wstring w_sep(&fs::path::preferred_separator);
    std::string sep(w_sep.begin(), w_sep.end());
#else
    std::string sep = &fs::path::preferred_separator;
#endif

    auto ims = sm.images();

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
        else if (name == "Image") {
            // Nothing to do here?
        }
        else {
            // add the specific detector info here!
            for (const auto &d : sm.stemDetectors())
                if (d.name == name)
                    settings["stem"]["detectors"][d.name] = JSONUtils::stemDetectorToJson(d);
            settings["microscope"].erase("alpha");
            settings["microscope"].erase("delta");
        }

        std::string out_name = out_path + sep + name;

        try {
            if (name == "EW") { // save amplitude and phase
                if (im.getSliceSize() % 2 != 0)
                    throw std::runtime_error("Attempting to save complex image with non equal real and imaginary parts.");

                auto im_d = im.getDimensions();

                Image<double> abs(im_d[0], im_d[1], im_d[2]);
                Image<double> arg(im_d[0], im_d[1], im_d[2]);

                for (int j = 0; j < im.getDepth(); j+=2)
                    for (int k = 0; k < im.getSliceSize(); k+=2) {
                        auto cval = std::complex<double>(im.getSliceRef(j)[k], im.getSliceRef(j)[k + 1]);
                        abs.getSliceRef(j)[k / 2] = std::abs(cval);
                        arg.getSliceRef(j)[k / 2] = std::arg(cval);
                    }


                fileio::SaveSettingsJson(out_name + "_amplitude.json", settings);
                fileio::SaveSettingsJson(out_name + "_phase.json", settings);

                saveTiffOutput(out_name+"_amplitude", abs, settings);
                saveTiffOutput(out_name+"_phase", arg, settings);
            } else {
                fileio::SaveSettingsJson(out_name + ".json", settings);
                saveTiffOutput(out_name, im, settings);
            }
        } catch (std::runtime_error &e) {
            std::cout << "Error saving image: " << e.what() << std::endl;
            CLOG(ERROR, "cmd") << "Could not save images: " << e.what();
        }
    }
}

int main(int argc, char *argv[])
{
    int verbose_flag = 0;
    int fix_cif = 0;
    total_pcnt = 0;
    slice_pcnt = 0;
    int c;

    std::string output_dir;
    std::string input_struct;
    std::string input_params;

    std::string device_options;

    std::vector<std::string> non_option_args;

    std::string size_arg, zone_arg, normal_arg, tilt_arg;

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
                        {"size",   required_argument, nullptr,       's'},
                        {"zone",   required_argument, nullptr,       'z'},
                        {"normal",   required_argument, nullptr,       'n'},
                        {"tilts",   required_argument, nullptr,       't'},
                        {"fix",   no_argument, &fix_cif,       1},
                        {"debug",  no_argument,       &verbose_flag, 1},
                        {nullptr, 0, nullptr, 0}
                };
        // getopt_long stores the option index here.
        int option_index = 0;
        c = getopt_long(argc, argv, "hvlo:c:d:s:z:n:t:", long_options, &option_index);

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
            case 's':
                size_arg = optarg;
                break;
            case 'z':
                zone_arg = optarg;
                break;
            case 'n':
                normal_arg = optarg;
                break;
            case 't':
                tilt_arg = optarg;
                break;
            case '?':
                // getopt_long already printed an error message.
                break;

            default:
                return 1;
        }
    }

    if (verbose_flag) {
        std::cout << "Debug flag is set" << std::endl << std::endl;
    }

    // get the non option args
    while (optind < argc)
        non_option_args.emplace_back(argv[optind++]);


    // check the options we need have at least been entered
    bool valid_flags = true;

    if (non_option_args.empty()) {
        std::cerr << "Require non-option argument as structure (.xyz or .cif) file" << std::endl;
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

    bool iscif = false;
    bool isxyz = false;
    
    CIF::SuperCellInfo sc_info;

    if (!non_option_args.empty()) {
        // this is where the input is actually set
        input_struct = non_option_args[0];
        isxyz = input_struct.compare(input_struct.size() - 4, 4, ".xyz") == 0;
        iscif = input_struct.compare(input_struct.size() - 4, 4, ".cif") == 0;

        if (!isxyz && !iscif) {
            std::cerr << "Require a .xyz or .cif file non-option argument. Instead got: " << input_struct << std::endl;
            valid_flags = false;
        }

        // Process all the .cif relevant options here
        if (iscif) {
            if (size_arg.empty()) {
                std::cerr << ".cif files require a size arg (-s, --size)" << std::endl;
                valid_flags = false;
            }

            if (zone_arg.empty()) {
                std::cerr << ".cif files require a zone axis arg (-z, --zone)" << std::endl;
                valid_flags = false;
            }

            double sx, sy, sz;
            if (!parseThreeCommaList(size_arg, sx, sy, sz)) {
                std::cerr << "Could not parse .cif size argument: " << size_arg << std::endl;
                valid_flags = false;
            } else
                sc_info.setWidths(sx, sy, sz);

            double zu, zv, zw;
            if (!parseThreeCommaList(zone_arg, zu, zv, zw)) {
                std::cerr << "Could not parse .cif size argument: " << zone_arg << std::endl;
                valid_flags = false;
            } else
                sc_info.setZoneAxis(zu, zv, zw);

            double nu, nv, nw;
            if (!normal_arg.empty()) {
                if (!parseThreeCommaList(normal_arg, nu, nv, nw)) {
                    std::cerr << "Could not parse .cif normal argument: " << normal_arg << std::endl;
                    valid_flags = false;
                } else
                    sc_info.setHorizontalAxis(nu, nv, nw);
            }

            double tx, ty, tz;
            if (!tilt_arg.empty()) {
                if (!parseThreeCommaList(tilt_arg, tx, ty, tz)) {
                    std::cerr << "Could not parse .cif tilt argument: " << tilt_arg << std::endl;
                    valid_flags = false;
                } else
                    sc_info.setTilts(tx, ty, tz);
            }

        } else if (isxyz) {
            if (!size_arg.empty() || !zone_arg.empty() || !normal_arg.empty() || !tilt_arg.empty())
                std::cerr << "WARNING: .cif options have been set for .xyz file, these will be ignored" << std::endl;
        }
    }

    if (!valid_flags)
        return 1;

    //
    // Set up the logging
    //
    // create a logger for our gui and simulation
    el::Loggers::getLogger("gui");
    el::Loggers::getLogger("sim");

    // create the conf to actually use of config
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(el::ConfigurationType::Enabled, "false"); // default to no logging

#ifdef _WIN32
    std::wstring w_sep(&fs::path::preferred_separator);
    std::string sep(w_sep.begin(), w_sep.end());
#else
    std::string sep = &fs::path::preferred_separator;
#endif

    if (verbose_flag) {
#ifdef _WIN32 // windows
        std::string appdata_loc = std::string(std::getenv("LOCALAPPDATA")) + sep + "PetersSoft" + sep + "clTEM";
#else // I only support linux (no apple stuff)
        // I think I am fine 'hard coding' the .config location
        std::string appdata_loc = std::string(std::getenv("HOME")) + sep + ".local" + sep + "share" + sep + "PetersSoft" + sep + "clTEM";
#endif

        // Get a writable location to save the log file
        std::string log_dir = appdata_loc + sep + "log.log";

        fs::path dir(appdata_loc);
        if (!fs::is_directory(dir)) {
            std::cout << "Log directory (" + appdata_loc + ") does not exist. Attempting to create..." << std::endl;
            bool good;
            try {
                good = fs::create_directory(dir);
            } catch (fs::filesystem_error& e) {
                std::cout << "Error making log directory. Exiting..." << std::endl;
                std::cout << e.what() << std::endl;
                return 1;
            }

            if (!good) {
                std::cout << "Error making log directory. Exiting..." << std::endl;
                CLOG(ERROR, "cmd") << "Error making output directory";
                return 1;
            }

            std::cout << "Successfully created folder" << std::endl;
        }


        defaultConf.setGlobally(el::ConfigurationType::Filename, log_dir);
        defaultConf.setGlobally(el::ConfigurationType::Format, "[%logger] %datetime (thread:%thread) %level - %func: %msg");
        defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
        defaultConf.setGlobally(el::ConfigurationType::ToFile, "true");
        defaultConf.setGlobally(el::ConfigurationType::MaxLogFileSize, "1073741824");
        defaultConf.setGlobally(el::ConfigurationType::Enabled, "true");
    }

    // set the config for the loggers
    el::Loggers::reconfigureAllLoggers(defaultConf);

    // this makes '%thread' show this string instead of a largely useless number
    el::Helpers::setThreadName("main-cmd");

    CLOG(INFO, "cmd") << "Logging set up and read to go!";

    //
    // On to setting up the simulation
    //

    std::cout << "Getting OpenCL devices:" << std::endl;

    std::vector<clDevice> device_list;

    try {
        device_list = getDevices(device_options);
    } catch (std::runtime_error &e) {
        std::cout << "Error finding OpenCL devices" << e.what() << std::endl;
        CLOG(ERROR, "cmd") << "Could not get OpenCL devices: " << e.what();
        return 1;
    }

    if (device_list.empty()) {
        std::cout << "No valid OpenCL device selected. Exiting..." << std::endl;
        CLOG(ERROR, "cmd") << "No valid OpenCL device selected";
        return 1;
    }

    std::cout << std::endl;
    // now have all our files/folders
    // need to check that they are all valid!

    // read the config file in
    nlohmann::json j;

    std::cout << "Config file: " << input_params << std::endl;
    try {
        j = fileio::OpenSettingsJson(input_params);
    } catch (...) {
        std::cout << "Error opening config file. Exiting..." << std::endl;
        CLOG(ERROR, "cmd") << "Error opening config file";
        return 1;
    }

    // make our manager...
    bool area_set;
    auto man_ptr = std::make_shared<SimulationManager>(JSONUtils::JsonToManager(j, area_set));

    // areas have been explicitly set in the config file, so we will use those (else the structure limits will be used)
    man_ptr->setMaintainAreas(area_set);

    // try to open the structure file...
    std::cout << "Structure file: " << input_struct << std::endl;
    try {
        if (iscif)
            man_ptr->setStructure(input_struct, sc_info, fix_cif==1);
        else
            man_ptr->setStructure(input_struct);
    } catch (const std::exception& e) {
        std::cout << "Error opening structure file: " << e.what() << std::endl;
        CLOG(ERROR, "cmd") << "Error opening structure file";
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
        bool good;
        try {
            good = fs::create_directory(dir);
        } catch (fs::filesystem_error& e) {
            std::cout << "Error making directory. Exiting..." << std::endl;
            CLOG(ERROR, "cmd") << "Error making output directory: " << e.what();
            std::cout << e.what() << std::endl;
            return 1;
        }

        if (!good) {
            std::cout << "Error making directory. Exiting..." << std::endl;
            CLOG(ERROR, "cmd") << "Error making output directory";
            return 1;
        }

        std::cout << "Successfully created folder" << std::endl;
    }

    // load external sources

    std::string exe_path_string;

#ifdef _WIN32
    // https://stackoverflow.com/a/13310600
    char exe_path[MAX_PATH];

    // When NULL is passed to GetModuleHandle, the handle of the exe itself is returned
    HMODULE hModule = GetModuleHandle(nullptr);
    if (hModule != nullptr) {
        // Use GetModuleFileName() with module handle to get the path
        GetModuleFileName(nullptr, exe_path, MAX_PATH);
        exe_path_string = std::string(dirname(exe_path));
    }
    else {
        std::cerr << "Cannot get executable path - Module handle is NULL" << std::endl ;
        CLOG(ERROR, "cmd") << "Cannot get executable path - Module handle is NULL";
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
    std::string params_path = exe_path_string + sep + "params";
    auto p_name = JSONUtils::readJsonEntry<std::string>(j, "potentials");
    man_ptr->setStructureParameters(p_name);

    for (const auto& params_file: boost::filesystem::directory_iterator(params_path))
        Utils::readParams(params_file.path().string());

    // check all our prerequisites here (some repeated?)
    try {
        Utils::checkSimulationPrerequisites(man_ptr, device_list);
    } catch (const std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    // sort plasmon stuff
    if (man_ptr->inelasticScattering()->plasmons()->enabled()) {
        int parts = man_ptr->totalParts();
        man_ptr->inelasticScattering()->plasmons()->initDepthVectors(parts);
        auto z_lims = man_ptr->simulationCell()->crystalStructure()->limitsZ();
        double thk = z_lims[1] - z_lims[0];

        bool valid = false;
        for (int i = 0; i < parts; ++i) {
            valid = man_ptr->inelasticScattering()->plasmons()->generateScatteringDepths(i, thk);

        if (!valid) {
            std::cout << "Could not generate valid plasmon configuration." << std::endl;
            return 1;
        }

        }
    }

    // open the kernels
    std::string kernel_path = exe_path_string + sep + "kernels";

    if (man_ptr->doublePrecisionEnabled()) {
        Kernels::atom_sort_d = Utils::resourceToChar(kernel_path, "atom_sort_d.cl");
        Kernels::band_limit_d = Utils::resourceToChar(kernel_path, "band_limit_d.cl");
        Kernels::band_pass_d = Utils::resourceToChar(kernel_path, "band_pass_d.cl");
        Kernels::ccd_dqe_d = Utils::resourceToChar(kernel_path, "ccd_dqe_d.cl");
        Kernels::ccd_ntf_d = Utils::resourceToChar(kernel_path, "ccd_ntf_d.cl");
        Kernels::complex_multiply_d = Utils::resourceToChar(kernel_path, "complex_multiply_d.cl");
        Kernels::ctem_image_d = Utils::resourceToChar(kernel_path, "ctem_image_d.cl");
        Kernels::fft_shift_d = Utils::resourceToChar(kernel_path, "fft_shift_d.cl");
        Kernels::init_plane_wave_d = Utils::resourceToChar(kernel_path, "init_plane_wave_d.cl");
        Kernels::init_probe_wave_d = Utils::resourceToChar(kernel_path, "init_probe_wave_d.cl");
        Kernels::transmission_potentials_full_3d_d = Utils::resourceToChar(kernel_path, "transmission_potentials_full_3d_d.cl");
        Kernels::transmission_potentials_projected_d = Utils::resourceToChar(kernel_path, "transmission_potentials_projected_d.cl");
        Kernels::propagator_d = Utils::resourceToChar(kernel_path, "propagator_d.cl");
        Kernels::sqabs_d = Utils::resourceToChar(kernel_path, "sqabs_d.cl");
        Kernels::sum_reduction_d = Utils::resourceToChar(kernel_path, "sum_reduction_d.cl");
        Kernels::bilinear_translate_d = Utils::resourceToChar(kernel_path, "bilinear_translate_d.cl");
        Kernels::complex_to_real_d = Utils::resourceToChar(kernel_path, "complex_to_real_d.cl");
    } else {
        Kernels::atom_sort_f = Utils::resourceToChar(kernel_path, "atom_sort_f.cl");
        Kernels::band_limit_f = Utils::resourceToChar(kernel_path, "band_limit_f.cl");
        Kernels::band_pass_f = Utils::resourceToChar(kernel_path, "band_pass_f.cl");
        Kernels::ccd_dqe_f = Utils::resourceToChar(kernel_path, "ccd_dqe_f.cl");
        Kernels::ccd_ntf_f = Utils::resourceToChar(kernel_path, "ccd_ntf_f.cl");
        Kernels::complex_multiply_f = Utils::resourceToChar(kernel_path, "complex_multiply_f.cl");
        Kernels::ctem_image_f = Utils::resourceToChar(kernel_path, "ctem_image_f.cl");
        Kernels::fft_shift_f = Utils::resourceToChar(kernel_path, "fft_shift_f.cl");
        Kernels::init_plane_wave_f = Utils::resourceToChar(kernel_path, "init_plane_wave_f.cl");
        Kernels::init_probe_wave_f = Utils::resourceToChar(kernel_path, "init_probe_wave_f.cl");
        Kernels::transmission_potentials_full_3d_f = Utils::resourceToChar(kernel_path, "transmission_potentials_full_3d_f.cl");
        Kernels::transmission_potentials_projected_f = Utils::resourceToChar(kernel_path, "transmission_potentials_projected_f.cl");
        Kernels::propagator_f = Utils::resourceToChar(kernel_path, "propagator_f.cl");
        Kernels::sqabs_f = Utils::resourceToChar(kernel_path, "sqabs_f.cl");
        Kernels::sum_reduction_f = Utils::resourceToChar(kernel_path, "sum_reduction_f.cl");
        Kernels::bilinear_translate_f = Utils::resourceToChar(kernel_path, "bilinear_translate_f.cl");
        Kernels::complex_to_real_f = Utils::resourceToChar(kernel_path, "complex_to_real_f.cl");
    }

    auto ccd_name = man_ptr->ccdName();

    if (ccd_name != "" && ccd_name != "Perfect") {
        std::string ccds_path = exe_path_string + sep + "ccds";
        std::vector<double> dqe, ntf;
        std::string name;
        Utils::ccdToDqeNtf(ccds_path, ccd_name + ".dat", name, dqe, ntf);
        CCDParams::addCCD(name, dqe, ntf);
    }

    // global because I am lazy (or smart?)
    out_path = output_dir;

    std::vector<std::shared_ptr<SimulationManager>> man_list;

    man_list.emplace_back(man_ptr);

    auto simRunner = std::make_shared<SimulationRunner>(man_list, device_list, man_ptr->doublePrecisionEnabled());

    simRunner->runSimulations();

    return 0;
}
