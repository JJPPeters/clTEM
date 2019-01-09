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
    Image() : width(0), height(0), pad_t(0), pad_l(0), pad_b(0), pad_r(0) {}
    Image(int w, int h, std::vector<T> d, int pt = 0, int pl = 0, int pb = 0, int pr = 0) : width(w), height(h),
                                                                                            data(d),
                                                                                            pad_t(pt), pad_l(pl),
                                                                                            pad_b(pb), pad_r(pr) {}
    Image(const Image<T>& rhs) : width(rhs.width), height(rhs.height), data(rhs.data), pad_t(rhs.pad_t),
                                                                                       pad_l(rhs.pad_l),
                                                                                       pad_b(rhs.pad_b),
                                                                                       pad_r(rhs.pad_r) {}

    Image<T>& operator=(const Image<T>& rhs) {
        width = rhs.width;
        height = rhs.height;
        data = rhs.data;
        pad_t = rhs.pad_t;
        pad_l = rhs.pad_l;
        pad_b = rhs.pad_b;
        pad_r = rhs.pad_r;
    }


    int width;
    int height;
    int pad_t, pad_l, pad_b, pad_r;
    std::vector<T> data;

    int getCroppedWidth() {return width - pad_l - pad_r;}
    int getCroppedHeight() {return height - pad_t - pad_b;}
    std::vector<T> getCropped() {
        std::vector<T> data_out(getCroppedWidth()*getCroppedHeight());
        int cnt = 0;
        for (int j = 0; j < height; ++j)
            if (j >= pad_b && j < (height - pad_t))
                for (int i = 0; i < width; ++i)
                    if (i >= pad_l && i < (width - pad_r))
                    {
                        int k = i + j * width;
                        data_out[cnt] = data[k];
                        ++cnt;
                    }
        return data_out;
    }
};

struct ComplexAberration {
    ComplexAberration() : Mag(0.0f), Ang(0.0f) {}
    ComplexAberration(float m, float a) : Mag(m), Ang(a) {}

    float Mag, Ang;

    std::complex<float> getComplex() {return std::polar(Mag, Ang);}
};

struct MicroscopeParameters {
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

    // Voltaage (kV) == (kg m^2 C^-1 s^-2)
    float Voltage;
    //Condenser aperture size (mrad)
    float Aperture;

    //Convergence angle (?)
    float Alpha;
    //Defocus spread (?)
    float Delta;

    //Calculate wavelength (Angstroms)
    float Wavelength() {
        return Constants::h * Constants::c / std::sqrt( Constants::eCharge * (Voltage * 1000.0f) * (2.0f * Constants::eMass * Constants::c*Constants::c + Constants::eCharge * ( Voltage * 1000.0f ) ) ) * 1e+10f;
    }

    // Interaction parameter (see Kirkland Eq. 5.6) (s^2 C m^-2 kg^-1 Angstrom^-1])
    float Sigma() {
        return (2 * Constants::Pi / (Wavelength() * (Voltage * 1000.0f))) * (Constants::eMass*Constants::c*Constants::c + Constants::eCharge * (Voltage * 1000.0f)) / (2 * Constants::eMass*Constants::c*Constants::c + Constants::eCharge * (Voltage * 1000.0f));
    }
};

struct SimulationArea {
    SimulationArea() : xStart(0.0f), xFinish(10.0f), yStart(0.0f), yFinish(10.0f), padding(0.f) {}

    SimulationArea(float xs, float xf, float ys, float yf, float pd = 0.f) : xStart(xs), xFinish(xf),
                                                                             yStart(ys), yFinish(yf),
                                                                             padding(pd) {}

    std::valarray<float> getRawLimitsX();
    std::valarray<float> getRawLimitsY();

    void setRawLimitsX(float start, float finish);
    void setRawLimitsY(float start, float finish);

    std::valarray<float> getCorrectedLimitsX();
    std::valarray<float> getCorrectedLimitsY();

    void setPadding(float pd) {padding = pd;}

protected:
    float xStart, xFinish, yStart, yFinish, padding;
};

struct StemDetector {
    StemDetector() : name("--"), inner(0.0f), outer(1.0f), xcentre(0.0f), ycentre(0.0f) {}
    StemDetector(std::string nm, float in, float out, float xc, float yc) : name(nm),
            inner(in), outer(out), xcentre(xc), ycentre(yc) {}

    std::string name;

    float inner, outer, xcentre, ycentre;
};

// TODO: is this class necessary, can we not just use the sim area one with the start and end points the same?
struct CbedPosition {
    CbedPosition() : xPos(0.0f), yPos(0.0f), padding(0.0f) {}
    CbedPosition(float _x, float _y, float _pd = 0.0f) : xPos(_x), yPos(_y), padding(_pd) {}

    float getXPos() {return xPos;}
    float getYPos() {return yPos;}
    float getPadding() {return padding;}

    void setXPos(float xp) {xPos = xp;}
    void setYPos(float yp) {yPos = yp;}
    void setPaddding(float pd) {padding = pd;}

    void setPos(float xp, float yp) {
        xPos = xp;
        yPos = yp;
    }

    SimulationArea getSimArea() {
        // pad equally on both sides
        return SimulationArea(xPos, xPos, yPos, yPos, padding);
    }

private:
    float xPos, yPos, padding;
};

struct StemArea : public SimulationArea {
    static const float DefaultScale;

    StemArea() : SimulationArea(), xPixels(64), yPixels(64) {}
    StemArea(float xs, float xf, float ys, float yf, int xp, int yp, float pd = 0.0f) : SimulationArea(xs, xf, ys, yf, pd),
                                                                                 xPixels(xp), yPixels(yp) {}

    bool setPxRangeX(float start, float finish, int px);
    bool setPxRangeY(float start, float finish, int px);

    void forcePxRangeX(float start, float finish, int px);
    void forcePxRangeY(float start, float finish, int px);

    void setPixelsX(int px) {xPixels = px;}
    void setPixelsY(int px) {yPixels = px;}

    float getStemPixelScaleX() { return (xFinish - xStart) / xPixels;}
    float getStemPixelScaleY() { return (yFinish - yStart) / yPixels;}

    int getPixelsX() {return xPixels;}
    int getPixelsY() {return yPixels;}
    float getPadding() {return padding;}

    float getScaleX();
    float getScaleY();

    unsigned int getNumPixels() {return xPixels * yPixels;}

private:
    unsigned int xPixels, yPixels;

};
#endif //COMMONSTRUCTS_H
