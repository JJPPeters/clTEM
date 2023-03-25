//
// Created by jon on 17/12/17. Lol
//

#ifndef CLTEM_FILEIO_H
#define CLTEM_FILEIO_H

#include <string>
#include <vector>
#include <array>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include "tiffio.h"

#include "json.hpp"

#include "stringutils.h"

namespace fileio //D:
{
    using json = nlohmann::json;

    void SaveSettingsJson(std::string filepath, json man);

    json OpenSettingsJson(std::string filepath);

//    template <typename T_out, typename T_in>
//    void SaveTiffStack(std::string folderpath, Image<T_in> data, unsigned int index_increment) {
//
////        if (!Utils::stringEndsWith(folderpath, sep))
////            folderpath += sep;
//
//        for (int i = 0; i < data.getDepth(); ++i) {
//            std::string out_file = folderpath + Utils::numToString(i);
//            SaveTiff(out_file, data.getSliceRef(i), data.getWidth(), data.getHeight());
//        }
//
//    }

    template <typename T_out, typename T_in>
    void SaveTiff(const std::string &filepath, std::vector<T_in> data, unsigned int size_x, unsigned int size_y)
    {
        if (size_x * size_y != data.size())
            throw std::runtime_error("Attempting to save image with incommensurate data size and image dimensions");

        //TODO: need to check for error on opening, or wa'evs
        TIFF* out(TIFFOpen(filepath.c_str(), "w"));

        if (!out)
            throw std::runtime_error("Could not open .tif file for saving: " + filepath);//aka tantrum

        TIFFSetField(out, TIFFTAG_IMAGEWIDTH, size_x);
        TIFFSetField(out, TIFFTAG_IMAGELENGTH, size_y);
        TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1);
        TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, sizeof(T_out)*8);
        TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, size_y);
        TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT);
        TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
        TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

        // virtually nothing supports 64-bit tiff so we will convert it here.
        std::vector<T_out> buffer(data.size());

        for (size_t i = 0; i < data.size(); ++i)
            buffer[i] = static_cast<T_out>(data[i]);

        if( (TIFFWriteEncodedStrip(out, 0, &buffer[0], sizeof(float)*buffer.size())) == -1)
            throw std::runtime_error("Unable to write data to .tif file");

        TIFFClose(out);
    }

    template <typename T_in>
    void SaveBmp(const std::string &filepath, std::vector<T_in> data, unsigned int size_x, unsigned int size_y)
    {
        // copied to a large extent from https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
        if (size_x * size_y != data.size())
            throw std::runtime_error("Attempting to save image with incommensurate data size and image dimensions");

        // convert the data to 0-255 range (char)

        T_in min = *std::min_element(data.begin(), data.end());
        T_in max = *std::max_element(data.begin(), data.end()); // because we will subtract min from everything, this is the new max
        max -= min;

        // need to pad the rows to be a multiple of 4
        unsigned int pad = 4 - (size_x % 4);
        if (pad == 4)
            pad = 0;

        unsigned int padded_x = size_x + pad;

        std::vector<unsigned char> data_out(padded_x*size_y);

        unsigned int ind = 0;
        for (unsigned int j = 0; j < size_y; ++j) {
            for (unsigned int i = 0; i < size_x; ++i) {
                // normalise data to 0 to 255 data range
                unsigned int in_ind = j * size_x + i;
                T_in t1 = data[in_ind] - min;
                double t2 = (double) t1 / (double) max;
                data_out[ind] = (unsigned char) std::round(t2 * 255);
                ++ind;
            }
            for (unsigned int i = 0; i < pad; ++i) {
                data_out[ind] = 0;
                ++ind;
            }
        }

        // info can be found here https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html
        std::array<unsigned char, 14> bmpfileheader{};//= {'B','M', 0,0,0,0, 0,0, 0,0, 0,0,0,0};
        std::array<unsigned char, 40> bmpinfoheader{};//= {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 8,0};
        std::fill(bmpfileheader.begin(), bmpfileheader.end(), 0); // make doubly sure everything else is 0
        std::fill(bmpinfoheader.begin(), bmpinfoheader.end(), 0); // make doubly sure everything else is 0

        bmpfileheader[0] = 'B';
        bmpfileheader[1] = 'M';

        // 54 is the header stuff, 1024 is the rgb-quad
        unsigned long filesize = 54 + padded_x*size_y + 1024;
        bmpfileheader[2] = (unsigned char)(filesize & 0xFF);
        bmpfileheader[3] = (unsigned char)((filesize >> 8) & 0xFF);
        bmpfileheader[4] = (unsigned char)((filesize >> 16) & 0xFF);
        bmpfileheader[5] = (unsigned char)((filesize >> 24) & 0xFF);

        unsigned int offset = 54+1024; // should always be the same, but just in case we change something
        bmpfileheader[10] = (unsigned char)(offset & 0xFF);
        bmpfileheader[11] = (unsigned char)((offset >> 8) & 0xFF);
        bmpfileheader[12] = (unsigned char)((offset >> 16) & 0xFF);
        bmpfileheader[13] = (unsigned char)((offset >> 24) & 0xFF);

        // used if you just initiate the vector to 0s
        bmpinfoheader[0] = 40;
        bmpinfoheader[12] = 1;
        bmpinfoheader[14] = 8;

        bmpinfoheader[4] = (unsigned char)(size_x & 0xFF);
        bmpinfoheader[5] = (unsigned char)((size_x >> 8) & 0xFF);
        bmpinfoheader[6] = (unsigned char)((size_x >> 16) & 0xFF);
        bmpinfoheader[7] = (unsigned char)((size_x >> 24) & 0xFF);

        bmpinfoheader[8] = (unsigned char)(size_y & 0xFF);
        bmpinfoheader[9] = (unsigned char)((size_y >> 8) & 0xFF);
        bmpinfoheader[10] = (unsigned char)((size_y >> 16) & 0xFF);
        bmpinfoheader[11] = (unsigned char)((size_y >> 24) & 0xFF);

        // save the 8 bit array
        std::ofstream file_out(filepath, std::ofstream::binary);

        if (!file_out.is_open())
            throw std::runtime_error("Could not open .bmp file for saving: " + filepath);

        // size in bytes is the array size here, would need to include the size of Type here too otherwise
        file_out.write((char*)&bmpfileheader[0], 14);
        file_out.write((char*)&bmpinfoheader[0], 40);

        auto a = (unsigned char) 0;
        for (unsigned int i = 0; i < 256; ++i) {
            auto rgb = (unsigned char) i;
            file_out << rgb << rgb << rgb << a;
        }

        file_out.write((char*)&data_out[0], padded_x*size_y);

        file_out.close();
    }

};


#endif //CLTEM_FILEIO_H
