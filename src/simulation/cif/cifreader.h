//
// Created by Jon on 21/08/2015.
//

#ifndef XYZ_CIFREADER_H
#define XYZ_CIFREADER_H

// for file input
#include <string>
#include <fstream>
#include <streambuf>
#include <regex>
#include <sstream>
#include <vector>

#include <Eigen/Dense>

#include "cifutilities.h"
#include "atomsite.h"
#include "symmetryoperation.h"
#include "unitcell.h"
#include "cellgeometry.h"

namespace CIF {
    class CIFReader {
    public:
        // constructor that takes filename
        explicit CIFReader(std::string filePath);

        // method to return a class instance that will contain the unit cell information
        UnitCell getUnitCell() { return UnitCell(cell, atomsites); }

        std::string getFilePath(){return file_path;}

    private:
        // class instance to hold unit cell information
        std::vector<Symmetry> symmetrylist;

        std::vector<AtomSite> atomsites;

        CellGeometry cell;

        std::string file_path;

        // methods to read
        // 1. atom positions
        // 2. symmetry
        // 3. basis geometry
        void readAtomPositions(const std::string &input);

        void readSymmetryOperations(const std::string &input);

        void readCellGeometry(const std::string &input);

    };
}

#endif //XYZ_CIFREADER_H
