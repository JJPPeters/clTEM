//
// Created by Jon on 23/08/2015.
//

#include "symmetryoperation.h"

void Symmetry::setOperation(int i, std::vector<std::string> factors)
{
    SymmetryOperation temp = SymmetryOperation(factors);

    if (i == 0)
        xOperation = temp;
    else if (i == 1)
        yOperation = temp;
    else if (i == 2)
        zOperation = temp;
    // TODO: else statement with error handling
}

SymmetryOperation Symmetry::getOperation(int i)
{
    if (i == 0)
        return xOperation;
    else if (i == 1)
        return yOperation;
    else if (i == 2)
        return zOperation;
    else
        throw std::runtime_error("Trying to get symmetry operation with unknown index: " + std::to_string(i));
}

SymmetryOperation::SymmetryOperation(std::vector<std::string> factors)
{
    xf = 0;
    yf = 0;
    zf = 0;
    c = 0;

    for (std::string term : factors)
    {
        if (term.back() == 'x')
        {
            term.pop_back();
            xf += fractionToDecimal(term, 1.0);
        }
        else if (term.back() == 'y')
        {
            term.pop_back();
            yf += fractionToDecimal(term, 1.0);
        }
        else if (term.back() == 'z')
        {
            term.pop_back();
            zf += fractionToDecimal(term, 1.0);
        }
        else
            c += fractionToDecimal(term, 0.0);
    }
}

double SymmetryOperation::applyOperation(double xin, double yin, double zin)
{
    return xf * xin + yf * yin + zf * zin + c;
}

double SymmetryOperation::applyOperation(std::vector<double> positions)
{
    return applyOperation(positions[0], positions[1], positions[2]);
}

double SymmetryOperation::fractionToDecimal(std::string fractionstring, double nullReturn)
{
    if (fractionstring == "")
        return nullReturn;

    if (fractionstring == "+")
        return 1.0;
    else if (fractionstring == "-")
        return -1.0;

    std::vector<std::string> fsplit = Utilities::split(fractionstring, '/');

    return std::stod(fsplit[0]) / std::stod(fsplit[1]);
}
