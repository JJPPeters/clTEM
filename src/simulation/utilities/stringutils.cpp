#include <stdexcept>
#include <algorithm>
#include "stringutils.h"

namespace Utils
{

    // https://stackoverflow.com/questions/236129/the-most-elegant-way-to-iterate-the-words-of-a-string
    // split a string by the whitespace
    std::vector<std::string> splitStringSpace(const std::string &in)
    {
        std::vector<std::string> out;
        std::istringstream iss(in);

        while (iss) {
            std::string subs;
            iss >> subs;
            out.emplace_back(subs);
        }

        return out;
    }

    // Taken from http://stackoverflow.com/a/6089413
    std::istream& safeGetline(std::istream& is, std::string& t)
    {
        t.clear();

        // The characters in the stream are read one-by-one using a std::streambuf.
        // That is faster than reading them one-by-one using the std::istream.
        // Code that uses streambuf this way must be guarded by a sentry object.
        // The sentry object performs various tasks,
        // such as thread synchronization and updating the stream state.

        std::istream::sentry se(is, true);
        std::streambuf* sb = is.rdbuf();

        for(;;) {
            int c = sb->sbumpc();
            switch (c) {
            case '\n':
                return is;
            case '\r':
                if(sb->sgetc() == '\n')
                    sb->sbumpc();
                return is;
            case std::streambuf::traits_type::eof():
                // Also handle the case when the last line has no line ending
                if(t.empty())
                    is.setstate(std::ios::eofbit);
                return is;
            default:
                t += (char)c;
            }
        }
    }

    std::string simModeToString(SimulationMode mode)
    {
        switch (mode)
        {
            case SimulationMode::CTEM:
                return "CTEM";
            case SimulationMode::STEM:
                return "STEM";
            case SimulationMode::CBED:
                return "CBED";
            default:
                return "None";
        }
    }



    std::string resourceToChar(std::string full_directory, std::string fileName)
    {
        std::ifstream inStream(full_directory + "/" + fileName);

        if (inStream.fail())
            throw std::runtime_error("Error opening resource file: " + full_directory + "/" + fileName);

        std::string fileContents((std::istreambuf_iterator<char>(inStream)), (std::istreambuf_iterator<char>()));

        inStream.close();

        return fileContents;
    }

    std::vector<double> paramsToVector(std::string full_directory, std::string fileName, unsigned int &row_count)
    {
        std::ifstream inStream(full_directory + "/" + fileName);

        if (inStream.fail())
            throw std::runtime_error("Error opening resource file: " + full_directory + "/" + fileName);

        // here we just want the number of rows (a.k.a. the number of atoms
        // https://stackoverflow.com/questions/3482064/counting-the-number-of-lines-in-a-text-file
        row_count = 0;
        std::string temp;
        while (std::getline(inStream, temp))
            ++row_count;

        // start back at the beginning to do the actual reading
        inStream.clear();
        inStream.seekg(0, std::ifstream::beg);

        std::vector<double> out;
        double p;

        while (inStream >> p)
            out.push_back(p);

        inStream.close();

        return out;
    }

    void ccdToDqeNtf(std::string full_directory, std::string fileName, std::string& name, std::vector<double>& dqe_io, std::vector<double>& ntf_io)
    {
        std::ifstream inStream(full_directory + "/" + fileName);

        if (inStream.fail())
            throw std::runtime_error("Error opening resource file: " + full_directory + "/" + fileName);

        std::string header_temp;
        std::string data_temp;
        double tmp;

        dqe_io = std::vector<double>();
        ntf_io = std::vector<double>();

        bool found_dqe = false;
        bool found_ntf = false;

        // first line is always the name;
        std::getline(inStream, name);

        while(std::getline(inStream, header_temp))
        {
            // clear whitespace, don't know how robust this is. See https://stackoverflow.com/questions/83439/remove-spaces-from-stdstring-in-c
            header_temp.erase(std::remove_if(header_temp.begin(), header_temp.end(), isspace), header_temp.end());

            if (header_temp == "DQE")
            {
                if (std::getline(inStream, data_temp))
                {
                    std::stringstream data_stream(data_temp);
                    while (data_stream >> tmp)
                        dqe_io.push_back(tmp);
                    if (dqe_io.size() == 725)
                        found_dqe = true;
                }
            }
            else if (header_temp == "NTF")
            {
                if (std::getline(inStream, data_temp))
                {
                    std::stringstream data_stream(data_temp);
                    while (data_stream >> tmp)
                        ntf_io.push_back(tmp);
                    if (ntf_io.size() == 725)
                        found_ntf = true;
                }
            }
        }

        if (!found_dqe || !found_ntf)
            throw std::runtime_error("Could not find DQE and NTF in file: " + fileName);
    }
}
