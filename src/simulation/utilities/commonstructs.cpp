#include "commonstructs.h"

//Constants
const float Constants::Pi = 3.141592653589793238462643383279502884f;
const float Constants::eMass = 9.10938291e-031f;
const float Constants::eMassEnergy = 510.99906f;
const float Constants::eCharge = 1.6021773e-019f;
const float Constants::h = 6.6262e-034f;
const float Constants::c = 299792458.0f;
const float Constants::a0 = 52.9177e-012f;
const float Constants::a0A = 52.9177e-002f;


std::valarray<float> SimulationArea::getRawLimitsX() {
    return {xStart, xFinish};
}

std::valarray<float> SimulationArea::getRawLimitsY() {
    return {yStart, yFinish};
}

void SimulationArea::setRawLimitsX(float start, float finish) {
    xStart = start;
    xFinish = finish;
}

void SimulationArea::setRawLimitsY(float start, float finish){
    yStart = start;
    yFinish = finish;
}

std::valarray<float> SimulationArea::getCorrectedLimitsX() {
    float range_x = xFinish - xStart;
    float range_y = yFinish - yStart;

    float range_max = std::max(range_x, range_y);
    float range_total = range_max + padding;

    // not get the difference to calculate how much needs to be added to either side
    float x_diff = (range_total - range_x) / 2.0f;

    // pad equally on both sides
    return {xStart - x_diff, xFinish + x_diff};
}

std::valarray<float> SimulationArea::getCorrectedLimitsY() {
    float range_x = xFinish - xStart;
    float range_y = yFinish - yStart;

    float range_max = std::max(range_x, range_y);
    float range_total = range_max + padding;

    // not get the difference to calculate how much needs to be added to either side
    float y_diff = (range_total - range_y) / 2.0f;

    // pad equally on both sides
    return {yStart - y_diff, yFinish + y_diff};
}


const float StemArea::DefaultScale = 0.2f;

bool StemArea::setPxRangeX(float start, float finish, int px)
{
//    if (isFixed)
//        return false;

    xStart = start;
    xFinish = finish;
    xPixels = px;

    return true;
}

bool StemArea::setPxRangeY(float start, float finish, int px)
{
//    if (isFixed)
//        return false;

    yStart = start;
    yFinish = finish;
    yPixels = px;

    return true;
}

//std::tuple<float, float, int> StemArea::getLimitsX()
//{
//    return std::tuple<float, float, int>(xStart, xFinish, xPixels);
//}
//
//std::tuple<float, float, int> StemArea::getLimitsY()
//{
//    return std::tuple<float, float, int>(yStart, yFinish, yPixels);
//}

void StemArea::forcePxRangeX(float start, float finish, int xp)
{
    xStart = start;
    xFinish = finish;
    xPixels = xp;
}

void StemArea::forcePxRangeY(float start, float finish, int yp)
{
    yStart = start;
    yFinish = finish;
    yPixels = yp;
}

//bool StemArea::setRangeX(float start, float finish, int px)
//{
//    if (isFixed)
//        return false;
//
//    float oldScale = getScaleX();
//
//    xStart = start;
//    xFinish = finish;
//
//    xPixels = (int)((xFinish-xStart) / oldScale);
//
//    if (xPixels < 1)
//        xPixels = 1;
//
//    return true;
//}
//
//bool StemArea::setRangeY(float start, float finish, int px)
//{
//    if (isFixed)
//        return false;
//
//    float oldScale = getScaleY();
//
//    yStart = start;
//    yFinish = finish;
//
//    yPixels = (int)((yFinish-yStart) / oldScale);
//
//    if (yPixels < 1)
//        yPixels = 1;
//
//    return true;
//}

float StemArea::getScaleX()
{
    return (xFinish - xStart) / (xPixels-1);
}

float StemArea::getScaleY()
{
    return (yFinish - yStart) / (yPixels-1);
}

//bool StemArea::setRangeXInsideSim(std::shared_ptr<SimulationArea> sa)
//{
//    auto sr = sa->getLimitsX();
//    float s = sr[0];
//    float f = sr[1];
//
//    if (isFixed)
//    {
//        // only update the regions outside the sim area
//        float oldScale = getScaleX();
//        bool changed = false;
//
//        if (yStart < s)
//        {
//            xStart = s;
//            changed = true;
//        }
//
//        if (xFinish > f)
//        {
//            xFinish = f;
//            changed = true;
//        }
//
//        if (changed)
//            xPixels = (int) ((xFinish - xStart) / oldScale);
//
//        return changed;
//    }
//
//    return setRangeX(s, f);
//}
//
//bool StemArea::setRangeYInsideSim(std::shared_ptr<SimulationArea> sa)
//{
//    auto sr = sa->getLimitsY();
//    float s = sr[0];
//    float f = sr[1];
//
//    if (isFixed)
//    {
//        // only update the regions outside the sim area
//        float oldScale = getScaleY();
//        bool changed = false;
//
//        if (yStart < s)
//        {
//            yStart = s;
//            changed = true;
//        }
//
//        if (yFinish > f)
//        {
//            yFinish = f;
//            changed = true;
//        }
//
//        if (changed)
//            yPixels = (int) ((yFinish - yStart) / oldScale);
//
//        return changed;
//    }
//
//    return setRangeY(s, f);
//}
