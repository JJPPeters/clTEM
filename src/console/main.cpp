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
//#include <file

int main(int argc, char *argv[])
{
    int verbose_flag = 0;
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
        c = getopt_long (argc, argv, "hvo:c:d:", long_options, &option_index);

        // Detect the end of the options.
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                break;
            case 'h':
                std::cout << "Help message goes here!" << std::endl;
                return 0;
            case 'v':
                std::cout << "Version message goes here!" << std::endl;
                return 0;
            case 'l':
                std::cout << "List OpenCL devices here!" << std::endl;
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

    if (non_option_args.empty())
    {
        std::cout << "Require non-option argument as structure (.xyz) file" << std::endl;
        valid_flags = false;
    }

    if (output_dir.empty())
    {
        std::cerr << "Require output directory argument (-o, --output)" << std::endl;
        valid_flags = false;
    }

    if (device_options.empty())
    {
        std::cerr << "Require OpenCL device(s) to be selected (-d, --device)" << std::endl;
        valid_flags = false;
    }

    if (non_option_args.size() > 1)
    {
        std::cerr << "Only expecting one non-option argument. Instead got:" << std::endl;
        for (std::string& s : non_option_args)
            std::cerr << s << std::endl;
        valid_flags = false;
    }

    // this is where the input is actually set
    input_struct = non_option_args[0];

    if (!valid_flags)
        return 0;

    std::cout << "Getting OpenCL devices:" << std::endl;

    std::vector<clDevice> device_list = getDevices(device_options);

    if (device_list.size() < 1) {
        std::cout << "No valid OpenCL device selected. Exiting..." << std::endl;
        return 1;
    }

    std::cout << std::endl;
    // now have all our files/folders
    // need to check that they are all valid!

    // read the config file in

    nlohmann::json j;

    if (input_params.empty())
        std::cout << "No configuration file provided. Using default settings..." << std::endl;
    else {
        std::cout << "Config file: " << input_params << std::endl;
        try {
            j = fileio::OpenSettingsJson(input_params);
        } catch (...) {
            std::cout << "Error opening config file. Exiting..." << std::endl;
            return 1;
        }
    }

    // make our manager...
    SimulationManager man = JSONUtils::JsonToManager(j);

    // try to open the structure file...
    std::cout << "Structure file: " << input_struct << std::endl;
    try {
        man.setStructure(input_struct);
    } catch (...) {
        std::cout << "Error opening structure file. Exiting..." << std::endl;
        return 1;
    }



    std::cout << "Output directory: " << output_dir << std::endl;
    boost::filesystem::path dir(output_dir);
    if (!boost::filesystem::is_directory(dir)) {
        std::cout << "Directory does not exist. Attempting to create..." << std::endl;
        bool good = true;
        try {
            good = boost::filesystem::create_directory(dir);
        } catch (boost::filesystem::filesystem_error& e) {
            std::cout << "Error making directory. Exiting..." << std::endl;
            std::cout << e.what() << std::endl;
            return 1;
        }

        if (!good) {
            std::cout << "Error making directory. Exiting..." << std::endl;
            return 1;
        }
    }

    return 0;
}
