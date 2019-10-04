#include "commonstructs.h"
#include <math.h>

//Constants
const double Constants::Pi = M_PI;
const double Constants::eMass = 9.10938291e-031;
const double Constants::eMassEnergy = 510.99906;
const double Constants::eCharge = 1.6021773e-019;
const double Constants::h = 6.6262e-034;
const double Constants::c = 299792458.0;
const double Constants::a0 = 52.9177e-012;
const double Constants::a0A = 52.9177e-002;


std::valarray<double> SimulationArea::getRawLimitsX() {
    return {xStart, xFinish};
}

std::valarray<double> SimulationArea::getRawLimitsY() {
    return {yStart, yFinish};
}

void SimulationArea::setRawLimitsX(double start, double finish) {
    xStart = start;
    xFinish = finish;
}

void SimulationArea::setRawLimitsY(double start, double finish){
    yStart = start;
    yFinish = finish;
}

std::valarray<double> SimulationArea::getCorrectedLimitsX() {
    double range_x = xFinish - xStart;
    double range_y = yFinish - yStart;

    double range_max = std::max(range_x, range_y);
    double range_total = range_max + padding;

    // not get the difference to calculate how much needs to be added to either side
    double x_diff = (range_total - range_x) / 2;

    // pad equally on both sides
    return {xStart - x_diff, xFinish + x_diff};
}

std::valarray<double> SimulationArea::getCorrectedLimitsY() {
    double range_x = xFinish - xStart;
    double range_y = yFinish - yStart;

    double range_max = std::max(range_x, range_y);
    double range_total = range_max + padding;

    // not get the difference to calculate how much needs to be added to either side
    double y_diff = (range_total - range_y) / 2;

    // pad equally on both sides
    return {yStart - y_diff, yFinish + y_diff};
}


const double StemArea::DefaultScale = 0.2;

bool StemArea::setPxRangeX(double start, double finish, int px)
{
//    if (isFixed)
//        return false;

    xStart = start;
    xFinish = finish;
    xPixels = px;

    return true;
}

bool StemArea::setPxRangeY(double start, double finish, int px)
{
    yStart = start;
    yFinish = finish;
    yPixels = px;

    return true;
}

void StemArea::forcePxRangeX(double start, double finish, int xp)
{
    xStart = start;
    xFinish = finish;
    xPixels = xp;
}

void StemArea::forcePxRangeY(double start, double finish, int yp)
{
    yStart = start;
    yFinish = finish;
    yPixels = yp;
}

double StemArea::getScaleX()
{
    return (xFinish - xStart) / xPixels;
}

double StemArea::getScaleY()
{
    return (yFinish - yStart) / yPixels;
}