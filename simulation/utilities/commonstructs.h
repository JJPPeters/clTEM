#ifndef COMMONSTRUCTS_H
#define COMMONSTRUCTS_H

#include <complex>
//#include <controls/imagetab.h>
#include "clwrapper.h"
#include <valarray>

struct Constants
{
    static const float Pi;
    // electron mass (kg)
    static const float eMass;
    // electron mass (keV)
    static const float eMassEnergy;
    // electron charge (C)
    static const float eCharge;
    // Planck's constant (Js)
    static const float h;
    // speed of light (m/s)
    static const float c;
    // Bohr radius (m)
    static const float a0;
    // Bohr radius (Angstrom)
    static const float a0A;
};

template<class T>
struct Image
{
    Image() : width(0), height(0) {}
    Image(int w, int h, std::vector<T> d) : width(w), height(h), data(d) {}
//    Image(const Image<T>& rhs) : width(rhs.width), height(rhs.height), data(rhs.data) {}
//    operator=(const Image<T>& rhs) {width = rhs.width; height = rhs.height; data = rhs.data;}
    int width;
    int height;
    std::vector<T> data;
};

struct ComplexAberration
{
    ComplexAberration() : Mag(0.0f), Ang(0.0f) {}
    ComplexAberration(float m, float a) : Mag(m), Ang(a) {}

    float Mag, Ang;

    std::complex<float> getComplex() {return std::polar(Mag, Ang);}
};

struct MicroscopeParameters
{
    MicroscopeParameters() : C10(0.0f), C30(0.0f), C50(0.0f), Voltage(1.0f), Aperture(1.0f), Alpha(1.0f), Delta(1.0f) {}

    // Defocus
    float C10;
    //Two-fold astigmatism
    ComplexAberration C12;
    //Coma
    ComplexAberration C21;
    //Three-fold astigmatism
    ComplexAberration C23;
    //Spherical
    float C30;

    ComplexAberration C32;

    ComplexAberration C34;

    ComplexAberration C41;

    ComplexAberration C43;

    ComplexAberration C45;
    //Fifth order spherical
    float C50;

    ComplexAberration C52;

    ComplexAberration C54;

    ComplexAberration C56;

    // Voltaage (kV)
    float Voltage;
    //Condenser aperture size (mrad)
    float Aperture;

    //Convergence angle (?)
    float Alpha;
    //Defocus spread (?)
    float Delta;

    //Calculate wavelength (Angstrom)
    float Wavelength()
    {
        return Constants::h * Constants::c / std::sqrt( Constants::eCharge * (Voltage * 1000.0f) * (2.0f * Constants::eMass * Constants::c*Constants::c + Constants::eCharge * ( Voltage * 1000.0f ) ) ) * 1e+010f;
    }

    // ?
    float Sigma()
    {
        return (2 * Constants::Pi / (Wavelength() * (Voltage * 1000.0f))) * (Constants::eMass*Constants::c*Constants::c + Constants::eCharge * (Voltage * 1000.0f)) / (2 * Constants::eMass*Constants::c*Constants::c + Constants::eCharge * (Voltage * 1000.0f));
    }
};

struct SimulationArea
{
    SimulationArea() : xStart(0.0f), xFinish(10.0f), yStart(0.0f), yFinish(10.0f), isFixed(false) {}

    std::valarray<float> getLimitsX();
    std::valarray<float> getLimitsY();

    bool setRangeX(float start, float finish);
    bool setRangeY(float start, float finish);

    void forceRangeX(float start, float finish);
    void forceRangeY(float start, float finish);

    void setFixed(bool f) {isFixed = f;}
    bool getIsFixed() {return isFixed;}

protected:
    float xStart, xFinish, yStart, yFinish;
    bool isFixed;
};

struct StemDetector
{
    StemDetector() : name("--"), inner(0.0f), outer(1.0f), xcentre(0.0f), ycentre(0.0f) {}
    StemDetector(std::string nm, float in, float out, float xc, float yc) : name(nm),
            inner(in), outer(out), xcentre(xc), ycentre(yc) {}

    std::string name;

    float inner, outer, xcentre, ycentre;
};

struct CbedPosition
{
    CbedPosition() : xPos(0.0f), yPos(0.0f) {}

    float getXPos() {return xPos;}
    float getYPos() {return yPos;}

    void setXPos(float xp) {xPos = xp;}
    void setYPos(float yp) {yPos = yp;}

    void setPos(float xp, float yp)
    {
        xPos = xp;
        yPos = yp;
    }

private:
    float xPos, yPos;
};

struct StemArea : public SimulationArea
{
    static const float DefaultScale;

    StemArea() : SimulationArea(), xPixels(50), yPixels(50) {}

//    bool setRangeX(float start, float finish);
//    bool setRangeY(float start, float finish);

    bool setPxRangeX(float start, float finish, int px);
    bool setPxRangeY(float start, float finish, int px);

    void forcePxRangeX(float start, float finish, int px);
    void forcePxRangeY(float start, float finish, int px);

    void setPixelsX(int px) {xPixels = px;}
    void setPixelsY(int px) {yPixels = px;}

    int getPixelsX() {return xPixels;}
    int getPixelsY() {return yPixels;}
//
//    std::tuple<float, float, int> getLimitsX();
//    std::tuple<float, float, int> getLimitsY();

//    std::valarray<float> getLimitsX();
//    std::valarray<float> getLimitsY();

    float getScaleX();
    float getScaleY();

//    void setFixed(bool f) {isFixed = f;}
//    bool getIsFixed() {return isFixed;}

//    bool setRangeXInsideSim(std::shared_ptr<SimulationArea> sa);
//    bool setRangeYInsideSim(std::shared_ptr<SimulationArea> sa);

    int getNumPixels() {return xPixels * yPixels;}

private:
//    float xStart, xFinish, yStart, yFinish;
    int xPixels, yPixels;
//    bool isFixed;
};
#endif //COMMONSTRUCTS_H
