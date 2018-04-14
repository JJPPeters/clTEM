#include "imageplotwidget.h"

ImagePlotWidget::ImagePlotWidget(QWidget *parent) : QCustomPlot(parent)
{
    setInteractions(QCP::iRangeZoom);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setFocusPolicy(Qt::StrongFocus);

    // this will remove all the crud but keep the grid lines
    xAxis->setSubTickPen(Qt::NoPen);
    xAxis->setTickPen(Qt::NoPen);
    xAxis->setTickLabels(false);
    xAxis->setBasePen(Qt::NoPen);
    yAxis->setSubTickPen(Qt::NoPen);
    yAxis->setTickPen(Qt::NoPen);
    yAxis->setTickLabels(false);
    yAxis->setBasePen(Qt::NoPen);

    // to get origin in centre
    xAxis->setRange(-500, 500);
    yAxis->setRange(-500, 500);

    matchPlotToPalette();

    axisRect()->setAutoMargins(QCP::msNone);
    axisRect()->setMinimumMargins(QMargins(0,0,0,0));
    axisRect()->setMargins(QMargins(0,0,0,0));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
}

bool ImagePlotWidget::event(QEvent *event) {
    // this might get spammed a bit, not sure if it is supposed to
    if (event->type() == QEvent::PaletteChange)
    {
        matchPlotToPalette();
        replot();
    }

    // very important or no other events will get through
    return QCustomPlot::event(event);
}

void ImagePlotWidget::matchPlotToPalette() {
    QPalette pal = qApp->palette();
    QPen axesPen = QPen(Qt::DashLine);
    axesPen.setColor(pal.color(QPalette::Mid));

    setBackground(qApp->palette().brush(QPalette::Background));
    xAxis->grid()->setPen(axesPen);
    yAxis->grid()->setPen(axesPen);

    QPen zeroPen = QPen(Qt::SolidLine);
    zeroPen.setWidth(2);
    zeroPen.setColor(pal.color(QPalette::Mid));

    xAxis->grid()->setZeroLinePen(zeroPen);
    yAxis->grid()->setZeroLinePen(zeroPen);
}

void ImagePlotWidget::SetImage(const std::vector<double> &image, const int sx, const int sy, IntensityScale scale, bool doReplot) {
    if (sx*sy != (int)image.size())
        throw std::runtime_error("Attempting to display image with size not matching given dimensions.");

    clearImage();

    AspectRatio = (double)sx/(double)sy;

//    rescaleAxes();
//    setImageRatio();
    resetAxes(false);

    ImageObject = new QCPColorMap(xAxis, yAxis);
    //addPlottable(ImageObject);
    ImageObject->setGradient(QCPColorGradient::gpGrayscale); // default
    ImageObject->setInterpolate(false);

    //check image is same size as dimensions given
    //check imageobject is not null?
    ImageObject->data()->setSize(sx, sy);
    ImageObject->data()->setRange(QCPRange(-(double)sx/2, (double)sx/2), QCPRange(-(double)sy/2, (double)sy/2));
    for (int xIndex=0; xIndex<sx; ++xIndex)
        for (int yIndex=0; yIndex<sy; ++yIndex)
        {
            if (scale == IntensityScale::Linear)
                ImageObject->data()->setCell(xIndex, yIndex, image[yIndex*sx+xIndex]);
            else
                ImageObject->data()->setCell(xIndex, yIndex, std::log10(1+image[yIndex*sx+xIndex])); // TODO: check image isn't negative (and so on...)
        }

    size_x = sx;
    size_y = sy;

    cropImage(false);

    ImageObject->rescaleDataRange(true); // TODO: maybe pass the true (to update scale better???)
    resetAxes(doReplot);

    haveImage = true;
}

void
ImagePlotWidget::SetImage(const std::vector<std::complex<double>> &image, const int sx, const int sy, ShowComplex show,
                          bool doReplot) {
    if (sx*sy != (int)image.size())
        throw std::runtime_error("Attempting to display image with size not matching given dimensions.");

    clearImage();

    AspectRatio = (double)sx/(double)sy;

//    resetAxes(false);
//    setImageRatio();
    resetAxes(false);

    ImageObject = new QCPColorMap(xAxis, yAxis);
    //addPlottable(ImageObject);
    ImageObject->setGradient(QCPColorGradient::gpGrayscale); // default
    ImageObject->setInterpolate(false);

    //check image is same size as dimensions given
    //check imageobject is not null?
    ImageObject->data()->setSize(sx, sy);
    ImageObject->data()->setRange(QCPRange(-(double)sx/2, (double)sx/2), QCPRange(-(double)sy/2, (double)sy/2));
    if (show == ShowComplex::Real)
    {
        for (int xIndex=0; xIndex<sx; ++xIndex)
            for (int yIndex=0; yIndex<sy; ++yIndex)
                ImageObject->data()->setCell(xIndex, yIndex, std::real(image[yIndex*sx+xIndex]));
    }
    else if (show == ShowComplex::Complex)
    {
        for (int xIndex=0; xIndex<sx; ++xIndex)
            for (int yIndex=0; yIndex<sy; ++yIndex)
                ImageObject->data()->setCell(xIndex, yIndex, std::imag(image[yIndex*sx+xIndex]));
    }
    else if (show == ShowComplex::Phase)
    {
        for (int xIndex=0; xIndex<sx; ++xIndex)
            for (int yIndex=0; yIndex<sy; ++yIndex)
                ImageObject->data()->setCell(xIndex, yIndex, std::arg(image[yIndex*sx+xIndex]));
    }
    else if (show == ShowComplex::Amplitude)
    {
        for (int xIndex=0; xIndex<sx; ++xIndex)
            for (int yIndex=0; yIndex<sy; ++yIndex)
                ImageObject->data()->setCell(xIndex, yIndex, std::abs(image[yIndex*sx+xIndex]));
    }
    else if (show == ShowComplex::PowerSpectrum)
    {
        for (int xIndex=0; xIndex<sx; ++xIndex)
            for (int yIndex=0; yIndex<sy; ++yIndex)
                ImageObject->data()->setCell(xIndex, yIndex, std::log10(1+std::abs(image[yIndex*sx+xIndex])));
    }

    size_x = sx;
    size_y = sy;

    cropImage(false);

    ImageObject->rescaleDataRange();
    resetAxes(doReplot);

    haveImage = true;
}

void ImagePlotWidget::DrawCircle(double x, double y, QColor colour, QBrush fill, double radius, Qt::PenStyle line,
                                 double thickness) {
    QCPItemEllipse* circle(new QCPItemEllipse(this));
    circle->setPen(QPen(colour, thickness, line));
    circle->setBrush(fill);
    circle->topLeft->setCoords(x-radius, y-radius);
    circle->bottomRight->setCoords(x+radius, y+radius);
    //addItem(circle);
    replot();
}

void
ImagePlotWidget::DrawRectangle(double t, double l, double b, double r, QColor colour, QBrush fill, double thickness) {
    QCPItemRect* rect(new QCPItemRect(this));
    rect->setPen(QPen(colour, thickness));
    rect->setBrush(fill);
    rect->topLeft->setCoords(l, t);
    rect->bottomRight->setCoords(r, b);
    //addItem(rect);
    replot();
}

void ImagePlotWidget::clearImage(bool doReplot) {
    haveImage = false;
    clearPlottables();
    if (doReplot)
        replot();
}

void ImagePlotWidget::clearAllItems(bool doReplot) {
    clearItems();
    if (doReplot)
        replot();
}

bool ImagePlotWidget::inAxis(double x, double y) {
    int bottom = ImageObject->data()->valueRange().lower;
    int top = ImageObject->data()->valueRange().upper;
    int left = ImageObject->data()->keyRange().lower;
    int right = ImageObject->data()->keyRange().upper;

    return x < right && x > left && y < top && y > bottom;
}

void ImagePlotWidget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event)
    setViewport(rect());
    setImageRatio(event->size().width(), event->size().height());
    replot(rpQueuedRefresh);
}

void ImagePlotWidget::setImageRatio() {
    setImageRatio(axisRect()->width(), axisRect()->height());
}

void ImagePlotWidget::setImageRatio(int axisWidth, int axisHeight) {
    // this case is needed as, when resizing the graph very fast, the values will switch and the calculation will be wrong.
    // this just sets the ratio to the intermediate point when the axes are equal, before then scaling to the correct values.

    if ( lastWidth <= AspectRatio*lastHeight && axisWidth > AspectRatio*axisHeight ) // plot WAS TALL
        yAxis->setRange(yAxis->range().center(), xAxis->range().size() / AspectRatio, Qt::AlignCenter);
    else if (  lastWidth >= AspectRatio*lastHeight && axisWidth < AspectRatio*axisHeight ) // plot WAS WIDE
        xAxis->setRange(xAxis->range().center(), AspectRatio*yAxis->range().size(), Qt::AlignCenter);

    lastWidth = axisWidth;
    lastHeight = axisHeight;

    if (axisWidth < AspectRatio*axisHeight) // plot is TALL
    {
        double newRange = xAxis->range().size()*(double)axisHeight / (double)axisWidth;
        yAxis->setRange(yAxis->range().center(), newRange, Qt::AlignCenter);
    }
    else if (AspectRatio*axisHeight < axisWidth) // plot is WIDE
    {
        double newRange = yAxis->range().size()*(double)axisWidth / (double)axisHeight;
        xAxis->setRange(xAxis->range().center(), newRange, Qt::AlignCenter);
    }
}

void ImagePlotWidget::SetColorLimits(double ul) {
    if (!haveImage)
        return;

    QCPRange lims(-ul, ul);
    ImageObject->setDataRange(lims);
    replot();
}

void ImagePlotWidget::SetColorMap(QCPColorGradient Map) {
    if (!haveImage)
        return;
    ImageObject->setGradient(Map);
    replot();
}

void ImagePlotWidget::resetAxes(bool doReplot) {
    if(!haveImage)
    {
        xAxis->setRange(-500, 500);
        yAxis->setRange(-500, 500);
    }
    //rescaleAxes(); //old way
    // use this instead to account for cropping
    if (crop_image) {
        xAxis->setRange(crop_l - size_x / 2, -crop_r + size_x / 2);
        yAxis->setRange(crop_t - size_y / 2, -crop_b + size_y / 2);
    }
    else{
        xAxis->setRange(-size_x / 2, size_x / 2);
        yAxis->setRange(-size_y / 2, size_y / 2);
    }

    setImageRatio();
    if (doReplot)
        replot();
}

void ImagePlotWidget::contextMenuRequest(QPoint pos) {
    QMenu* menu = new QMenu(this);

    menu->addAction("Reset zoom", this, SLOT(resetAxes()));

    QMenu* save_menu = new QMenu("Export...", menu);

    save_menu->addAction("Data", this, SLOT(exportTiff()));
    save_menu->addAction("RGB", this, SLOT(exportBmp()));

    menu->addMenu(save_menu);

    menu->popup(mapToGlobal(pos));
}

void ImagePlotWidget::cropImage(bool doReplot) {
    if (!crop_image) {
        ImageObject->data()->clearAlpha();
        return;
    }

    // do the cropping if we get haven't returned yet
    for (int ind = 0; ind < size_x*size_y; ++ind)
    {
        int i = ind % size_x;
        int j = ind / size_x;

        if (j < crop_b || j >= (size_y - crop_t) || i < crop_l || i >= (size_x - crop_r)) {
            ImageObject->data()->setAlpha(i, j, 0);
        }
    }

    if (doReplot)
        replot();
}

void ImagePlotWidget::exportTiff() {
    emit saveDataClicked();
}

void ImagePlotWidget::exportBmp() {
    emit saveImageClicked();
}