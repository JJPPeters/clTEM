#include <utility>

#ifndef COMMONSTRUCTS_H
#define COMMONSTRUCTS_H

#include <complex>
#include "clwrapper.h"
#include <valarray>

struct Constants
{
    static const double Pi;
    // electron mass (kg)
    static const double eMass;
    // electron mass (keV)
    static const double eMassEnergy;
    // electron charge (C)
    static const double eCharge;
    // Planck's constant (Js)
    static const double h;
    // speed of light (m/s)
    static const double c;
    // Bohr radius (m)
    static const double a0;
    // Bohr radius (Angstrom)
    static const double a0A;
};

template<class T>
struct Image
{
    Image() : width(0), height(0), pad_t(0), pad_l(0), pad_b(0), pad_r(0) {}
    Image(unsigned int w, unsigned int h, std::vector<T> d, unsigned int pt = 0, unsigned int pl = 0, unsigned int pb = 0, unsigned int pr = 0) : width(w), height(h),
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

        return *this;
    }


    unsigned int width;
    unsigned int height;
    unsigned int pad_t, pad_l, pad_b, pad_r;
    std::vector<T> data;

    unsigned int getCroppedWidth() {return width - pad_l - pad_r;}
    unsigned int getCroppedHeight() {return height - pad_t - pad_b;}
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
    ComplexAberration(double m, double a) : Mag(m), Ang(a) {}

    double Mag, Ang;

    std::complex<double> getComplex() {return std::polar(Mag, Ang);}
};

struct MicroscopeParameters {
    MicroscopeParameters() : C10(0.0f), C30(0.0f), C50(0.0f), Voltage(1.0f), Aperture(1.0f), Alpha(1.0f), Delta(1.0f) {}

    // Defocus
    double C10;
    //Two-fold astigmatism
    ComplexAberration C12;
    //Coma
    ComplexAberration C21;
    //Three-fold astigmatism
    ComplexAberration C23;
    //Spherical
    double C30;

    ComplexAberration C32;

    ComplexAberration C34;

    ComplexAberration C41;

    ComplexAberration C43;

    ComplexAberration C45;
    //Fifth order spherical
    double C50;

    ComplexAberration C52;

    ComplexAberration C54;

    ComplexAberration C56;

    // Voltage (kV) == (kg m^2 C^-1 s^-2)
    double Voltage;
    //Condenser aperture size (mrad)
    double Aperture;

    //Convergence angle (?)
    double Alpha;
    //Defocus spread (?)
    double Delta;

    //Calculate wavelength (Angstroms)
    double Wavelength() {
        return Constants::h * Constants::c / std::sqrt( Constants::eCharge * (Voltage * 1000) * (2 * Constants::eMass * Constants::c*Constants::c + Constants::eCharge * ( Voltage * 1000 ) ) ) * 1e+10;
    }

    // Interaction parameter (see Kirkland Eq. 5.6) (s^2 C m^-2 kg^-1 Angstrom^-1])
    double Sigma() {
        return (2 * Constants::Pi / (Wavelength() * (Voltage * 1000))) * (Constants::eMass*Constants::c*Constants::c + Constants::eCharge * (Voltage * 1000)) / (2 * Constants::eMass*Constants::c*Constants::c + Constants::eCharge * (Voltage * 1000));
    }
};

struct StemDetector {
    StemDetector() : name("--"), inner(0.0), outer(1.0), xcentre(0.0), ycentre(0.0) {}
    StemDetector(std::string nm, double in, double out, double xc, double yc) : name(std::move(nm)),
            inner(in), outer(out), xcentre(xc), ycentre(yc) {}

    std::string name;

    double inner, outer, xcentre, ycentre;
};

struct SimulationArea {
    SimulationArea() : xStart(0.0), xFinish(10.0), yStart(0.0), yFinish(10.0), padding(0.0) {}

    SimulationArea(double xs, double xf, double ys, double yf, double pd = 0.f) : xStart(xs), xFinish(xf),
                                                                                  yStart(ys), yFinish(yf),
                                                                                  padding(pd) {}

    std::valarray<double> getRawLimitsX();
    std::valarray<double> getRawLimitsY();

    void setRawLimitsX(double start, double finish);
    void setRawLimitsY(double start, double finish);

    std::valarray<double> getCorrectedLimitsX();
    std::valarray<double> getCorrectedLimitsY();

    void setPadding(double pd) {padding = pd;}

protected:
    double xStart, xFinish, yStart, yFinish, padding;
};

// TODO: is this class necessary, can we not just use the sim area one with the start and end points the same?
struct CbedPosition {
    CbedPosition() : xPos(0.0), yPos(0.0), padding(0.0) {}
    CbedPosition(double _x, double _y, double _pd = 0.0) : xPos(_x), yPos(_y), padding(_pd) {}

    double getXPos() {return xPos;}
    double getYPos() {return yPos;}
    double getPadding() {return padding;}

    void setXPos(double xp) {xPos = xp;}
    void setYPos(double yp) {yPos = yp;}
    void setPaddding(double pd) {padding = pd;}

    void setPos(double xp, double yp) {
        xPos = xp;
        yPos = yp;
    }

    SimulationArea getSimArea() {
        // pad equally on both sides
        return SimulationArea(xPos, xPos, yPos, yPos, padding);
    }

private:
    double xPos, yPos, padding;
};

struct StemArea : public SimulationArea {
    static const double DefaultScale;

    StemArea() : SimulationArea(), xPixels(64), yPixels(64) {}
    StemArea(double xs, double xf, double ys, double yf, unsigned int xp, unsigned int yp, double pd = 0.0) : SimulationArea(xs, xf, ys, yf, pd),
                                                                                 xPixels(xp), yPixels(yp) {}

    bool setPxRangeX(double start, double finish, int px);
    bool setPxRangeY(double start, double finish, int px);

    void forcePxRangeX(double start, double finish, int px);
    void forcePxRangeY(double start, double finish, int px);

    void setPixelsX(unsigned int px) {xPixels = px;}
    void setPixelsY(unsigned int py) {yPixels = py;}

    double getStemPixelScaleX() { return (xFinish - xStart) / xPixels;}
    double getStemPixelScaleY() { return (yFinish - yStart) / yPixels;}

    unsigned int getPixelsX() {return xPixels;}
    unsigned int getPixelsY() {return yPixels;}
    double getPadding() {return padding;}

    double getScaleX();
    double getScaleY();

    unsigned int getNumPixels() {return xPixels * yPixels;}

    SimulationArea getPixelSimArea(int pixel) {
        // convert pixel id to x, y position
        unsigned int x_px = pixel % xPixels;
        unsigned int y_px = (unsigned int) std::floor(pixel / xPixels);

        float xPos = getRawLimitsX()[0] + getScaleX() * x_px;
        float yPos = getRawLimitsY()[0] + getScaleY() * y_px;
        // pad equally on both sides
        return SimulationArea(xPos, xPos, yPos, yPos, padding);
    }

private:
    unsigned int xPixels, yPixels;

};
#endif //COMMONSTRUCTS_H
