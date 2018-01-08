//
// Created by jon on 17/12/17.
//

#ifndef CLTEM_FILEIO_H
#define CLTEM_FILEIO_H

#include <string>
#include <vector>
#include <stdexcept>
#include "tiffio.h"

namespace fileio
{

    template <typename T_out, typename T_in>
    void SaveTiff(std::string filepath, std::vector<T_in> data, int size_x, int size_y)
    {
        if (size_x * size_y != data.size())
            throw std::runtime_error("Attempting to save image with incommensurate data size and image dimensions");

        //TODO: need to check for error on opening
        TIFF* out(TIFFOpen(filepath.c_str(), "w"));

        if (!out)
            throw std::runtime_error("Could not open .tif file for saving: " + filepath);

        TIFFSetField(out, TIFFTAG_IMAGEWIDTH, size_x);
        TIFFSetField(out, TIFFTAG_IMAGELENGTH, size_y);
        TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1);
        TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, sizeof(T_out)*8);
        TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, size_y);
//       TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT);
        TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
        TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

        // virtually nothing supports 64-bit tiff so we will convert it here.
        std::vector<T_out> buffer(data.size());

        for (int i = 0; i < data.size(); ++i)
            buffer[i] = static_cast<T_out>(data[i]);

        tsize_t image_s;
        if( (image_s = TIFFWriteEncodedStrip(out, 0, &buffer[0], sizeof(float)*buffer.size())) == -1)
            throw std::runtime_error("Unable to write data to .tif file");

        (void)TIFFClose(out);
    }

};


#endif //CLTEM_FILEIO_H
