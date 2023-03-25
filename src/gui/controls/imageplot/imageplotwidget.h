#ifndef IMAGEPLOT
#define IMAGEPLOT

#include <complex>
#include <memory>
#include <cmath>

#include <qcustomplot.h>

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
    void saveDataClicked(bool);
    void saveImageClicked(bool);
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
    void getCurrentData(std::vector<T>& out, int& sx, int& sy) {
        return getData(out, sx, sy, current_slice);
    }

    template <typename T>
    void getData(std::vector<T>& out, int& sx, int& sy, int slice) {

        if (is_complex) {
            sx = data_complex.getWidth(crop_image);
            sy = data_complex.getHeight(crop_image);
            out = std::vector<T>(sx*sy);

            // get cropped data
            auto temp = calculateComplexData(slice, crop_image);
            for (size_t i = 0; i < temp.size(); ++i)
                out[i] = static_cast<T>(temp[i]);
        } else {
            sx = data_real.getWidth(crop_image);
            sy = data_real.getHeight(crop_image);
            out = std::vector<T>(sx*sy);

            // get cropped data
            auto temp = data_real.getWeightedSlice(slice, crop_image);

            size_t s_out = out.size();
            size_t s_in = temp.size();

            if (s_out != s_in)
                throw std::runtime_error("Getting data of incorrect size");

            for (size_t i = 0; i < temp.size(); ++i)
                out[i] = static_cast<T>(temp[i]);
        }

    }
//
//    template <typename T>
//    void getData(std::vector<T>& out, int& sx, int& sy)
//    {
//        int c_l = 0;
//        int c_r = 0;
//        int c_t = 0;
//        int c_b = 0;
//
//        if (crop_image)
//        {
//            c_t = crop_t;
//            c_l = crop_l;
//            c_b = crop_b;
//            c_r = crop_r;
//        }
//
//        sx = ImageObject->data()->keySize() - c_l - c_r;
//        sy = ImageObject->data()->valueSize() - c_t - c_b;
//
//        out = std::vector<T>(sx*sy);
//
//        int cnt = 0;
//        for (int j = c_b; j < full_size_y-c_t; ++j)
//            for (int i = c_l; i < full_size_x-c_r; ++i)
//            {
//                out[cnt] = static_cast<T>(ImageObject->data()->cell(i, j));
//                ++cnt;
//            }
//    }

    bool isComplex() {return is_complex;}

private:
    QCPColorMap *ImageObject = nullptr;

    bool haveImage = false;

    ZeroPosition zero_pos;

    IntensityScale int_scale;

    double AspectRatio = 1;

    bool crop_image = false;

    unsigned int current_slice = 0;

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
    void exportTiffStack();

    void exportBmp();
    void exportBmpStack();

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

        data_real = img;
        data_complex = Image<std::complex<double>>();

        current_slice = img.getDepth() - 1;

        SetImageGeneric(current_slice, z_x, z_y, sc_x, sc_y, intensity_scale, zp, redraw, reset);
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

        data_real = Image<double>();
        data_complex = img;

        current_slice = img.getDepth() - 1;

        SetImageGeneric(current_slice, z_x, z_y, sc_x, sc_y, intensity_scale, zp, redraw, reset);
    }

    void setComplexDisplay(ShowComplex show_c, bool redraw = true, bool reset = false) {
        if (!haveImage)
            return;

        complex_type = show_c;

        SetImageGeneric(current_slice, zero_x, zero_y, scale_x, scale_y, int_scale, zero_pos, redraw, reset);
    }

    void setSlice(unsigned int slice = 0) {
        std::vector<double> im_d;
        if (is_complex && data_complex.getDepth() > 1 && slice < data_complex.getDepth()) {
            im_d = calculateComplexData(slice);
        } else if (!is_complex && data_real.getDepth() > 1 && slice < data_real.getDepth()) {
            im_d = data_real.getSliceRef(slice);
        } else {
            return;
        }

        current_slice = slice;

        SetImageData(im_d, true, false);
    }

    unsigned int getSliceCount() {
        if (is_complex)
            return data_complex.getDepth();
        else
            return data_real.getDepth();
    }

private:
    bool is_complex;

    ShowComplex complex_type;

    Image<double> data_real;
    Image<std::complex<double>> data_complex; // only used when we have a complex image

    std::vector<double> calculateComplexData(unsigned int slice = 0, bool crop = false) {
        int sz = data_complex.getSliceSize(crop);
        std::vector<double> im_d(sz);

        std::vector<std::complex<double>> im_c = data_complex.getWeightedSlice(slice, crop);

        if (complex_type == ShowComplex::Real) {
            for (int i = 0; i < sz; ++i)
                im_d[i] = std::real(im_c[i]);
        } else if (complex_type == ShowComplex::Imag) {
            for (int i = 0; i < sz; ++i)
                im_d[i] = std::imag(im_c[i]);
        } else if (complex_type == ShowComplex::Amplitude) {
            for (int i = 0; i < sz; ++i)
                im_d[i] = std::abs(im_c[i]);
        } else if (complex_type == ShowComplex::Phase) {
            for (int i = 0; i < sz; ++i)
                im_d[i] = std::arg(im_c[i]);
        }

        return im_d;
    }

    void SetImageGeneric(unsigned int slice,
                         double z_x, double z_y,
                         double sc_x, double sc_y,
                         IntensityScale intensity_scale,
                         ZeroPosition zp,
                         bool redraw, bool reset)
    {
        if (is_complex) {
            auto im_d = calculateComplexData(slice);

            std::valarray<unsigned int> pd = data_complex.getPadding();
            crop_t = pd[0];
            crop_l = pd[1];
            crop_b = pd[2];
            crop_r = pd[3];

            std::valarray<unsigned int> sz = data_complex.getDimensions();
            full_size_x = sz[0];
            full_size_y = sz[1];

            crop_size_x = data_complex.getCroppedWidth();
            crop_size_y = data_complex.getCroppedHeight();

            scale_x = sc_x;
            scale_y = sc_y;
            zero_x = z_x;
            zero_y = z_y;
            zero_pos = zp;
            int_scale = intensity_scale;

            SetImageData(im_d, redraw, reset);
        } else {
            auto im_d = data_real.getSliceRef(slice);

            std::valarray<unsigned int> pd = data_real.getPadding();
            crop_t = pd[0];
            crop_l = pd[1];
            crop_b = pd[2];
            crop_r = pd[3];

            std::valarray<unsigned int> sz = data_real.getDimensions();
            full_size_x = sz[0];
            full_size_y = sz[1];

            crop_size_x = data_real.getCroppedWidth();
            crop_size_y = data_real.getCroppedHeight();

            scale_x = sc_x;
            scale_y = sc_y;
            zero_x = z_x;
            zero_y = z_y;
            zero_pos = zp;
            int_scale = intensity_scale;

            SetImageData(im_d, redraw, reset);
        }
    }

    void SetImagePlot(const std::vector<double> &image, bool redraw);

    void SetImageData(const std::vector<double> &image, bool redraw, bool reset);

};

#endif // IMAGEPLOT

