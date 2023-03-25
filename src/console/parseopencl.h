//
// Created by jon on 14/04/18.
//

#ifndef CLTEM_GUI_PARSEOPENCL_H
#define CLTEM_GUI_PARSEOPENCL_H

#include <clwrapper/cldevice.h>
#include <vector>
#include <sstream>

std::vector<clDevice> getDevices(std::string d_str)
{

    std::vector<clDevice> devices;

    // need to parse the device stuff!
    if (d_str == "default")
    {
        devices = OpenCL::GetDeviceList(Device::DeviceType::DEFAULT);
        std::cout << "No OpenCL device(s) set, using default device(s)" << std::endl;
    }
    else if (d_str == "all")
    {
        devices = OpenCL::GetDeviceList(Device::DeviceType::All);
        std::cout << "Using all available OpenCL devices" << std::endl;
    }
    else if (d_str == "gpus")
    {
        devices = OpenCL::GetDeviceList(Device::DeviceType::GPU);
        std::cout << "Using all available GPU OpenCL devices" << std::endl;
    }
    else if (d_str == "cpus")
    {
        devices = OpenCL::GetDeviceList(Device::DeviceType::CPU);
        std::cout << "Using all available CPU OpenCL devices" << std::endl;
    }
    else if (d_str == "gpu")
    {
        devices = OpenCL::GetDeviceList(Device::DeviceType::GPU);
        if (devices.empty())
            throw std::runtime_error("Could not get GPU OpenCL device");
        devices.erase(devices.begin()+1, devices.end());
        std::cout << "Picking a GPU OpenCL devices" << std::endl;
    }
    else if (d_str == "cpu")
    {
        devices = OpenCL::GetDeviceList(Device::DeviceType::CPU);
        if (devices.empty())
            throw std::runtime_error("Could not get CPU OpenCL device");
        devices.erase(devices.begin()+1, devices.end());
        std::cout << "Picking a CPU OpenCL devices" << std::endl;
    }
    else
    {
        // need to parse commas and colons
        // remove whitespace
        d_str.erase(std::remove(d_str.begin(), d_str.end(), ' '), d_str.end());

        // split our string into parts separated by commas
        std::stringstream ss(d_str);
        std::vector<std::string> parts;
        while( ss.good() )
        {
            std::string substr;
            getline( ss, substr, ',' );
            parts.emplace_back( substr );
        }

        std::vector<clDevice> dev_all = OpenCL::GetDeviceList(Device::DeviceType::All);

        if (dev_all.empty())
            throw std::runtime_error("Could not get OpenCL devices to choose from");

        for (auto& p : parts)
        {
            std::stringstream pss(p);
            unsigned int pid;
            unsigned int did;

            std::vector<std::string> sub_parts;
            while( pss.good() )
            {
                std::string substr;
                getline( pss, substr, ':' );
                sub_parts.emplace_back( substr );
            }

            bool valid = true;

            if (sub_parts.size() != 2)
                valid = false;

            try {
                pid = std::stoi(sub_parts[0]);
                did = std::stoi(sub_parts[1]);
            } catch (std::invalid_argument& e)
            {
                valid = false;
            }

            if (!valid) {
                std::cout << "Could not parse device identifier: " << p << ". Ignoring it..." << std::endl;
                continue;
            }

            bool exists = false;

            for (auto& d: dev_all)
                if (d.GetDeviceNumber() == did && d.GetPlatformNumber() == pid) {
                    devices.emplace_back(d);
                    exists = true;
                }

            if (!exists)
                std::cout << "Could not find an OpenCL device with platform id: " << pid << " and device id: " << did << ". Ignoring it..." << std::endl;
        }
    }

    for (size_t i = 0; i < devices.size(); ++i) {
        std::cout << "Platform: " << devices[i].GetPlatformNumber() << ", device: " << devices[i].GetDeviceNumber() << std::endl;
        std::cout << "\t" << devices[i].GetPlatformName() << " - " << devices[i].GetDeviceName() << std::endl;
    }

    if (devices.empty())
        throw std::runtime_error("Could not get OpenCL devices...");

    return devices;

}

#endif //CLTEM_GUI_PARSEOPENCL_H
