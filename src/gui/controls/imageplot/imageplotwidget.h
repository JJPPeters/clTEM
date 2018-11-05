#ifndef IMAGEPLOT
#define IMAGEPLOT

#include <complex>
#include <memory>
#include <cmath>

#include "qcustomplot.h"

#include <iostream>
#include <fstream>

#include <QtWidgets/QWidget>
#include <utilities/commonstructs.h>
#include <map>

enum ShowComplex {
    Real,
    Imag,
    Phase,
    Amplitude };

enum IntensityScale {
    Linear,
    Log };

enum ZeroPosition {
    Centre,
    BottomLeft };

typedef std::map<std::string, std::map<std::string, std::string>>::iterator it_type;

class ImagePlotWidget : public QCustomPlot
{
    Q_OBJECT

signals:
    void saveDataClicked();
    void saveImageClicked();
    void mouseHoverEvent(double, double);

public:
    explicit ImagePlotWidget(QWidget *parent);

    // this is used to update the colours by filtering the events
    bool event(QEvent *event) override;

    void matchPlotToPalette();

    void DrawCircle(double x, double y, QColor colour = Qt::red, QBrush fill = QBrush(Qt::red), double radius = 2, Qt::PenStyle line = Qt::SolidLine, double thickness = 2);

    void DrawRectangle(double t, double l, double b, double r, QColor colour = Qt::red, QBrush fill = QBrush(Qt::NoBrush), double thickness = 2);

    void clearImage(bool doReplot = true);

    void clearAllItems(bool doReplot = true);

    bool inAxis(double x, double y);

    bool getCropImage() {return crop_image;}
    void setCropImage(bool do_crop, bool redraw = false, bool rescale = false)
    {
        crop_image = do_crop;

        if (!haveImage)
            return;

        cropImage(false);

        if (rescale)
            resetAxes(redraw);
        else if (redraw)
            replot();
    }

    template <typename T>
    void getData(std::vector<T>& out, int& sx, int& sy)
    {
        int c_l = 0;
        int c_r = 0;
        int c_t = 0;
        int c_b = 0;

        if (crop_image)
        {
            c_t = crop_t;
            c_l = crop_l;
            c_b = crop_b;
            c_r = crop_r;
        }

        sx = ImageObject->data()->keySize() - c_l - c_r;
        sy = ImageObject->data()->valueSize() - c_t - c_b;

        out = std::vector<T>(sx*sy);

        int cnt = 0;
        for (int j = c_b; j < full_size_y-c_t; ++j)
            for (int i = c_l; i < full_size_x-c_r; ++i)
            {
                out[cnt] = static_cast<T>(ImageObject->data()->cell(i, j));
                ++cnt;
            }
    }

    bool isComplex() {return is_complex;}

private:
    QCPColorMap *ImageObject = nullptr;

    bool haveImage = false;

    ZeroPosition zero_pos;

    IntensityScale int_scale;

    double AspectRatio = 1;

    bool crop_image = false;

    int full_size_x, full_size_y, crop_size_x, crop_size_y;
    int crop_t, crop_l, crop_b, crop_r;

    double scale_x, scale_y;
    double zero_x, zero_y;

    int lastWidth, lastHeight;

    void resizeEvent(QResizeEvent* event);

    void setImageRatio();

    // Basically a reimplementation of setScalRatio() but for both axes
    void setImageRatio(int axisWidth, int axisHeight);

    void cropImage(bool doReplot = true);

    void mouseMoveEvent(QMouseEvent* event) {
        if (haveImage) {
            auto p = event->pos();
            double x = xAxis->pixelToCoord(p.x());
            double y = yAxis->pixelToCoord(p.y());

            emit mouseHoverEvent(x, y);
        }
        QCustomPlot::mouseMoveEvent(event);
    }

    void resetAxes(bool doReplot = true);

public slots:
    void SetColorLimits(double ul);

    void SetColorMap(QCPColorGradient Map);

    void resetAxes_slot() {resetAxes(true);} // this is purely for the slot...

    void exportTiff();

    void exportBmp();

private slots:
    void contextMenuRequest(QPoint pos);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Image display stuff here
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    template <typename T>
    void SetImage(Image<T> img,
                          double z_x = 0.0, double z_y = 0.0,
                          double sc_x = 1.0, double sc_y = 1.0,
                          IntensityScale intensity_scale = IntensityScale::Linear,
                          ZeroPosition zp = ZeroPosition::BottomLeft,
                          bool redraw = true, bool reset = true)
    {
        is_complex = false;
        SetImageGeneric(img.data, img.width, img.height, img.pad_t, img.pad_l, img.pad_b, img.pad_r,
                        z_x, z_y, sc_x, sc_y, intensity_scale, zp, redraw, reset);
    }

    template <typename T>
    void SetComplexImage(Image<std::complex<T>> img,
                                 double z_x = 0.0,double z_y = 0.0,
                                 double sc_x = 1.0, double sc_y = 1.0,
                                 IntensityScale intensity_scale = IntensityScale::Linear,
                                 ShowComplex show_comp = ShowComplex::Amplitude,
                                 ZeroPosition zp = ZeroPosition::BottomLeft,
                                 bool redraw = true, bool reset = true)
    {
        complex_type = show_comp;
        is_complex = true;
        data_complex = img.data;

        auto im_d = calculateComplexData();

        SetImageGeneric(im_d, img.width, img.height, img.pad_t, img.pad_l, img.pad_b, img.pad_r,
                        z_x, z_y, sc_x, sc_y, intensity_scale, zp, redraw, reset);
    }

    void setComplexDisplay(ShowComplex show_c, bool redraw = true, bool reset = false) {
        if (!haveImage)
            return;

        complex_type = show_c;

        auto im_d = calculateComplexData();
        SetImageGeneric(im_d, full_size_x, full_size_y,
                        crop_t, crop_l, crop_b, crop_r,
                        zero_x, zero_y, scale_x, scale_y, int_scale, zero_pos, redraw, reset);
    }

private:
    bool is_complex = false;

    ShowComplex complex_type;

    std::vector<std::complex<float>> data_complex; // only used when we have a complex image

    std::vector<float> calculateComplexData() {
        std::vector<float> im_d(data_complex.size());

        if (complex_type == ShowComplex::Real) {
            for (int i = 0; i < data_complex.size(); ++i)
                im_d[i] = std::real(data_complex[i]);
        } else if (complex_type == ShowComplex::Imag) {
            for (int i = 0; i < data_complex.size(); ++i)
                im_d[i] = std::imag(data_complex[i]);
        } else if (complex_type == ShowComplex::Amplitude) {
            for (int i = 0; i < data_complex.size(); ++i)
                im_d[i] = std::abs(data_complex[i]);
        } else if (complex_type == ShowComplex::Phase) {
            for (int i = 0; i < data_complex.size(); ++i)
                im_d[i] = std::arg(data_complex[i]);
        }

        return im_d;
    }

    template <typename T>
    void SetImageGeneric(std::vector<T> img, int sx, int sy,
                         int pad_t, int pad_l, int pad_b, int pad_r,
                         double z_x, double z_y,
                         double sc_x, double sc_y,
                         IntensityScale intensity_scale,
                         ZeroPosition zp,
                         bool redraw, bool reset)
    {
        // free up this complex data if we aren't going to use it
        if (!is_complex)
            data_complex.clear();

        std::vector<double> im_d(img.size());
        for (int i = 0; i < img.size(); ++i)
            im_d[i] = static_cast<double>(img[i]);
        crop_t = pad_t;
        crop_l = pad_l;
        crop_b = pad_b;
        crop_r = pad_r;
        scale_x = sc_x;
        scale_y = sc_y;
        zero_x = z_x;
        zero_y = z_y;
        zero_pos = zp;
        int_scale = intensity_scale;
        SetImageData(im_d, sx, sy, intensity_scale, redraw, reset);
    }

    void SetImagePlot(const std::vector<double> &image, int sx, int sy, IntensityScale intensity_scale, bool redraw);

    void SetImageData(const std::vector<double> &image, int sx, int sy, IntensityScale intensity_scale, bool redraw, bool reset);

};

#endif // IMAGEPLOT

