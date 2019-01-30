//
// Created by Jon on 23/08/2015.
//

#ifndef XYZ_SYMMETRYOPERATION_H
#define XYZ_SYMMETRYOPERATION_H

#include <string>
#include <vector>

#include "cifutilities.h"

namespace CIF {
    class SymmetryOperation {
        friend class Symmetry;

    protected:
        SymmetryOperation() {};
    public:
        double xf, yf, zf, c;

        SymmetryOperation(std::vector<std::string> factors);

        double applyOperation(double xin, double yin, double zin);

        double applyOperation(std::vector<double> positions);

    private:
        double fractionToDecimal(std::string fractionstring, double nullReturn);

    };

// Small class just to hold the symmetry operation for all directions
    class Symmetry {
    public:

        Symmetry() {}

//    void setXOperation(std::vector<std::string> factors);
//    void setYOperation(std::vector<std::string> factors);
//    void setZOperation(std::vector<std::string> factors);
        SymmetryOperation getOperation(int i);

        void setOperation(int i, std::vector<std::string> factors);

    private:
        SymmetryOperation xOperation, yOperation, zOperation;
    };
}

#endif //XYZ_SYMMETRYOPERATION_H
