#ifndef IMAGEPLOT
#define IMAGEPLOT

#include <complex>
#include <memory>
#include <cmath>

#include "controls/imageplot/qcustomplot.h"

#include <iostream>
#include <fstream>

#include <QWidget>

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

public:
    ImagePlotWidget(QWidget *parent);

    ~ImagePlotWidget() {}

    // this is used to update the colours by filtering the events
    bool event(QEvent *event);

    void matchPlotToPalette();

    template <typename T>
    void SetImageTemplate(const std::vector<T>& image, const int sx, const int sy, IntensityScale scale = IntensityScale::Linear, bool doReplot = true)
    {
        std::vector<double> im_d(image.size());
        for (int i = 0; i < image.size(); ++i)
            im_d[i] = (double) image[i];
        SetImage(im_d, sx, sy, scale, doReplot);
    }

    void SetImage(const std::vector<double>& image, const int sx, const int sy, IntensityScale scale = IntensityScale::Linear, bool doReplot = true);

    void SetImage(const std::vector<std::complex<double>>& image, const int sx, const int sy, ShowComplex show, bool doReplot = true);

    void DrawCircle(double x, double y, QColor colour = Qt::red, QBrush fill = QBrush(Qt::red), double radius = 2, Qt::PenStyle line = Qt::SolidLine, double thickness = 2);

    void DrawRectangle(double t, double l, double b, double r, QColor colour = Qt::red, QBrush fill = QBrush(Qt::NoBrush), double thickness = 2);

    void clearImage(bool doReplot = true);

    void clearAllItems(bool doReplot = true);

    bool inAxis(double x, double y);

private:
    QCPColorMap *ImageObject;

    bool haveImage = false;

    double AspectRatio = 1;

    int size_x, size_y;

    int lastWidth, lastHeight;

    void resizeEvent(QResizeEvent* event);

    void setImageRatio();

    // Basically a reimplementation of setScalRatio() but for both axes
    void setImageRatio(int axisWidth, int axisHeight);

    //There is always an extra pixel it seems so I rewrote this to compensate
//    bool saveRastered(const QString &fileName, int width, int height, double scale, const char *format, int quality = -1)
//    {
//      QPixmap buffer = toPixmap(width, height+1, scale);
//      QPixmap cropped = buffer.copy(0, 0, width, height);
//      if (!buffer.isNull())
//        return cropped.save(fileName, format, quality);
//      else
//        return false;
//    }

public slots:
    void SetColorLimits(double ul);

    void SetColorMap(QCPColorGradient Map);

//    void ExportSelector(QString directory, QString filename, int choice)
//    {
//        QString filepath = QDir(directory).filePath(filename);
//
//        if (choice == 0)
//        {
//            filepath += ".tif";
//            ExportImage(filepath, false);
//        }
//        else if (choice == 1)
//        {
//            filepath += ".tif";
//            ExportData(filepath);
//        }
//        else if (choice == 2)
//        {
//            filepath += ".bin";
//            ExportBinary(filepath);
//        }
//    }

    void ResetAxes();

private slots:
    void contextMenuRequest(QPoint pos);

//    void ExportImage()
//    {
//        QSettings settings;
//
//        // get path
//        QString filepath = QFileDialog::getSaveFileName(this, "Save image", settings.value("dialog/currentSavePath").toString(), "TIFF (*.tif)");
//
//        if (filepath.isEmpty())
//            return;
//
//        QFileInfo temp_file(filepath);
//        settings.setValue("dialog/currentSavePath", temp_file.path());
//
//        ExportImage(filepath);
//    }

//    void ExportImage(QString filepath, bool doReplot = true)
//    {
//        std::string format = "TIFF";
//
//        QCPRange xr = xAxis->range();
//        QCPRange yr = yAxis->range();
//
//        // the Axes are all screwed but this seems to work
//        // needed to export the right area
//        xAxis->setRange(QCPRange(-size_x/2, size_x/2));
//        yAxis->setRange(QCPRange(-size_y/2-1, size_y/2+1));
//        saveRastered(filepath, size_x, size_y, 1.0, format.c_str());
//
//        xAxis->setRange(xr);
//        yAxis->setRange(yr);
//
//        if(doReplot)
//            replot();
//    }

//    void ExportData()
//    {
//        QSettings settings;
//
//        // get path
//        QString filepath = QFileDialog::getSaveFileName(this, "Save image", settings.value("dialog/currentSavePath").toString(), "TIFF (*.tif)");
//
//        if (filepath.isEmpty())
//            return;
//
//        QFileInfo temp_file(filepath);
//        settings.setValue("dialog/currentSavePath", temp_file.path());
//
//        ExportData(filepath);
//    }

//    void ExportData(QString filepath)
//    {
//       //TODO: need to check for error on opening
//       TIFF* out(TIFFOpen(filepath.toStdString().c_str(), "w"));
//
//       if (!out)
//           return;
//
//       TIFFSetField(out, TIFFTAG_IMAGEWIDTH, size_x);
//       TIFFSetField(out, TIFFTAG_IMAGELENGTH, size_y);
//       TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1);
//       TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, sizeof(float)*8);
//       TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
//       TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, size_y);
//       TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT);
//       TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
//       TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
//       TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
//
//       // virtually nothing supports 64-bit tiff so we will convert it here.
//       std::vector<float> buffer(size_x*size_y);
//       double* data_temp = ImageObject->data()->getDataArray();
//
//       for (int i = 0; i < size_x*size_y; ++i)
//           buffer[i] = (float)data_temp[i];
//
//       tsize_t image_s;
//       if( (image_s = TIFFWriteEncodedStrip(out, 0, &buffer[0], sizeof(float)*size_x*size_y)) == -1)
//            std::cerr << "Unable to write tif file" << std::endl;
//
//       (void)TIFFClose(out);
//    }

//    void ExportBinary()
//    {
//        QSettings settings;
//
//        // get path
//        QString filepath = QFileDialog::getSaveFileName(this, "Save image", settings.value("dialog/currentSavePath").toString(), "Binary (*.bin)");
//
//        if (filepath.isEmpty())
//            return;
//
//        QFileInfo temp_file(filepath);
//        settings.setValue("dialog/currentSavePath", temp_file.path());
//
//        ExportBinary(filepath);
//    }
//
//    void ExportBinary(QString filepath)
//    {
//        std::ofstream out(filepath.toStdString(), std::ios::out | std::ios::binary);
//        if(!out)
//            std::cerr << "Unable to write binary file" << std::endl;
//
//        out.write((char *) ImageObject->data()->getDataArray(), size_x*size_y*sizeof(double));
//
//        out.close();
//    }
};

#endif // IMAGEPLOT

