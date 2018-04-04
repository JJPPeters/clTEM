#ifndef IMAGEPLOT
#define IMAGEPLOT

#include <complex>
#include <memory>
#include <cmath>

#include "controls/imageplot/qcustomplot.h"

#include <iostream>
#include <fstream>

#include <QWidget>
#include <simulation/utilities/commonstructs.h>

enum ShowComplex {
        Real,
        Complex,
        Phase,
        Amplitude,
        PowerSpectrum };

enum IntensityScale {
    Linear,
    Log };

typedef std::map<std::string, std::map<std::string, std::string>>::iterator it_type;

class ImagePlotWidget : public QCustomPlot
{
    Q_OBJECT

signals:
    void saveDataClicked();
    void saveImageClicked();

public:
    ImagePlotWidget(QWidget *parent);

    ~ImagePlotWidget() {}

    // this is used to update the colours by filtering the events
    bool event(QEvent *event);

    void matchPlotToPalette();

    template <typename T>
    void SetImageTemplate(Image<T> img, IntensityScale scale = IntensityScale::Linear, bool doReplot = true)
    {
        std::vector<double> im_d(img.data.size());
        for (int i = 0; i < img.data.size(); ++i)
            im_d[i] = (double) img.data[i];
        crop_t = img.pad_t;
        crop_l = img.pad_l;
        crop_b = img.pad_b;
        crop_r = img.pad_r;
        SetImage(im_d, img.width, img.height, scale, doReplot);
    }

    void DrawCircle(double x, double y, QColor colour = Qt::red, QBrush fill = QBrush(Qt::red), double radius = 2, Qt::PenStyle line = Qt::SolidLine, double thickness = 2);

    void DrawRectangle(double t, double l, double b, double r, QColor colour = Qt::red, QBrush fill = QBrush(Qt::NoBrush), double thickness = 2);

    void clearImage(bool doReplot = true);

    void clearAllItems(bool doReplot = true);

    bool inAxis(double x, double y);

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
        for (int j = c_b; j < size_y-c_t; ++j)
            for (int i = c_l; i < size_x-c_r; ++i)
            {
                out[cnt] = static_cast<T>(ImageObject->data()->cell(i, j));
                ++cnt;
            }
    }

private:
    QCPColorMap *ImageObject;

    bool haveImage = false;

    double AspectRatio = 1;

    bool crop_image = false;

    int size_x, size_y;
    int crop_t, crop_l, crop_b, crop_r;

    void SetImage(const std::vector<double>& image, const int sx, const int sy, IntensityScale scale = IntensityScale::Linear, bool doReplot = true);

    void SetImage(const std::vector<std::complex<double>>& image, const int sx, const int sy, ShowComplex show, bool doReplot = true);

    int lastWidth, lastHeight;

    void resizeEvent(QResizeEvent* event);

    void setImageRatio();

    // Basically a reimplementation of setScalRatio() but for both axes
    void setImageRatio(int axisWidth, int axisHeight);

    void cropImage(bool doReplot = true);

public slots:
    void SetColorLimits(double ul);

    void SetColorMap(QCPColorGradient Map);

    void resetAxes(bool doReplot = true);

    void exportTiff();

    void exportBmp();

private slots:
    void contextMenuRequest(QPoint pos);

};

#endif // IMAGEPLOT

