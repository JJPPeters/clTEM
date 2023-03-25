#include "imageplotwidget.h"

ImagePlotWidget::ImagePlotWidget(QWidget *parent) : QCustomPlot(parent)
{
    setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);
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

    // this is what really does the magic
    xAxis->setOffset(100);
    yAxis->setOffset(100);

    // to get origin in centre
    xAxis->setRange(-500, 500);
    yAxis->setRange(-500, 500);

    matchPlotToPalette();
    is_complex = false;

    axisRect()->setAutoMargins(QCP::msNone);
    // minus 10 means we can't accidentally scroll on it...
    axisRect()->setMinimumMargins(QMargins(0,0,0,0));
    axisRect()->setMargins(QMargins(0,0,0,0));

    connect(this, &ImagePlotWidget::customContextMenuRequested, this, &ImagePlotWidget::contextMenuRequest);
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

    setBackground(qApp->palette().brush(QPalette::Window));
    xAxis->grid()->setPen(axesPen);
    yAxis->grid()->setPen(axesPen);

    QPen zeroPen = QPen(Qt::SolidLine);
    zeroPen.setWidth(2);
    zeroPen.setColor(pal.color(QPalette::Mid));

    xAxis->grid()->setZeroLinePen(zeroPen);
    yAxis->grid()->setZeroLinePen(zeroPen);
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

    if ( lastWidth < AspectRatio*lastHeight && axisWidth >= AspectRatio*axisHeight ) // plot WAS TALL
        yAxis->setRange(yAxis->range().center(), xAxis->range().size() / AspectRatio, Qt::AlignCenter);
    else if (  lastWidth > AspectRatio*lastHeight && axisWidth <= AspectRatio*axisHeight ) // plot WAS WIDE
        xAxis->setRange(xAxis->range().center(), AspectRatio*yAxis->range().size(), Qt::AlignCenter);

    lastWidth = axisWidth;
    lastHeight = axisHeight;

    if (axisWidth < AspectRatio*axisHeight) // plot is TALL
    {
        auto old_range = xAxis->range().size();
        auto new_ratio = (double)axisHeight / (double)axisWidth;
        double newRange = old_range * new_ratio;
        yAxis->setRange(yAxis->range().center(), newRange, Qt::AlignCenter);
    }
    else if (AspectRatio*axisHeight < axisWidth) // plot is WIDE
    {
        auto old_range = yAxis->range().size();
        auto new_ratio = (double)axisWidth / (double)axisHeight;
        double newRange = old_range * new_ratio;
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
    if(!haveImage) {
        xAxis->setRange(-500, 500);
        yAxis->setRange(-500, 500);
    }
    else {
        auto kr = ImageObject->data()->keyRange();
        auto vr = ImageObject->data()->valueRange();

        if (crop_image) {
            xAxis->setRange(scale_x*(crop_l-0.5) + kr.lower, -scale_x*(crop_r-0.5) + kr.upper);
            yAxis->setRange(scale_y*(crop_b-0.5) + vr.lower, -scale_y*(crop_t-0.5) + vr.upper);
        } else {
            xAxis->setRange(kr.lower-scale_x*0.5, kr.upper+scale_x*0.5);
            yAxis->setRange(vr.lower-scale_y*0.5, vr.upper+scale_y*0.5);
        }
    }

    setImageRatio();
    if (doReplot)
        replot();
}

void ImagePlotWidget::contextMenuRequest(QPoint pos) {
    QMenu* menu = new QMenu(this);

    menu->addAction("Reset zoom", this, &ImagePlotWidget::resetAxes_slot);

    if (haveImage) {
        QMenu *save_menu_single = new QMenu("Export displayed...", menu);

        save_menu_single->addAction("Data", this, &ImagePlotWidget::exportTiff);
        save_menu_single->addAction("RGB", this, &ImagePlotWidget::exportBmp);

        menu->addMenu(save_menu_single);

        if ((is_complex && data_complex.getDepth() > 1) || (!is_complex && data_real.getDepth() > 1)) {
            QMenu *save_menu_stack = new QMenu("Export stack...", menu);

            save_menu_stack->addAction("Data", this, &ImagePlotWidget::exportTiffStack);
            save_menu_stack->addAction("RGB", this, &ImagePlotWidget::exportBmpStack);

            menu->addMenu(save_menu_stack);
        }
    }

    menu->popup(mapToGlobal(pos));
}

void ImagePlotWidget::cropImage(bool doReplot) {
    ImageObject->data()->clearAlpha();

    if (crop_image) { // do the cropping if we get haven't returned yet
        for (int ind = 0; ind < full_size_x * full_size_y; ++ind) {
            int i = ind % full_size_x;
            int j = ind / full_size_x;

            if (j < crop_b || j >= (full_size_y - crop_t) || i < crop_l || i >= (full_size_x - crop_r)) {
                ImageObject->data()->setAlpha(i, j, 0);
            }
        }
        AspectRatio = (double)crop_size_x/(double)crop_size_y;
    } else
        AspectRatio = (double)full_size_x/(double)full_size_y;

    if (doReplot)
        replot();
}

void ImagePlotWidget::exportTiff() {
    emit saveDataClicked(false);
}

void ImagePlotWidget::exportTiffStack() {
    emit saveDataClicked(true);
}

void ImagePlotWidget::exportBmp() {
    emit saveImageClicked(false);
}

void ImagePlotWidget::exportBmpStack() {
    emit saveImageClicked(true);
}

void
ImagePlotWidget::SetImageData(const std::vector<double> &image, bool redraw, bool reset) {

    // simple check that all our data is compatible
    if (full_size_x*full_size_y != (int)image.size())
        throw std::runtime_error("Attempting to display image with size not matching given dimensions.");

    if(!ImageObject) {
        SetImagePlot(image, redraw); // this function creates what is needed, then calls this function
        return;
    }

    ImageObject->data()->setSize(full_size_x, full_size_y);

    double r_x_low, r_x_high, r_y_low, r_y_high;

    if (zero_pos == ZeroPosition::Centre) {
        // these plus and minus ones may only work for the even images we have
        r_x_low = -scale_x * ((double) full_size_x + 1) / 2.0;
        r_y_low = -scale_y * ((double) full_size_y + 1) / 2.0;
        r_x_high = scale_x * ((double) full_size_x - 1) / 2.0;
        r_y_high = scale_y * ((double) full_size_y - 1) / 2.0;
    } else {
        r_x_low = zero_x;
        r_y_low = zero_y;
        r_x_high = zero_x + scale_x * ((double) full_size_x - 1.0);
        r_y_high = zero_y + scale_y * ((double) full_size_y - 1.0);
    }

    ImageObject->data()->setRange(QCPRange(r_x_low, r_x_high), QCPRange(r_y_low, r_y_high));

    if (int_scale != IntensityScale::Linear)
        ImageObject->setDataScaleType(QCPAxis::ScaleType::stLogarithmic);
    else
        ImageObject->setDataScaleType(QCPAxis::ScaleType::stLinear);

    // set data and calculate our contrast limits

    double minHeight = std::numeric_limits<double>::infinity();
    double maxHeight = -1 * std::numeric_limits<double>::infinity();

    // need to get weightings from appropriate image (real or complex)

    for (int xIndex=0; xIndex<full_size_x; ++xIndex)
        for (int yIndex=0; yIndex<full_size_y; ++yIndex) {
            int ind = yIndex * full_size_x + xIndex;
            double val = image[ind];
            double wt;
            if (is_complex)
                wt = data_complex.getWeightingVal(ind);
            else
                wt = data_real.getWeightingVal(ind);

            val /= wt;

            ImageObject->data()->setCell(xIndex, yIndex, val);

            if (wt > 0.0) {
                if (val > maxHeight)
                    maxHeight = val;
                if (val < minHeight)
                    minHeight = val;
            }
        }

    // calculate the aspect ration (so we can maintain it)
    if (crop_image) {
        AspectRatio = (double)crop_size_x/(double)crop_size_y;
    } else
        AspectRatio = (double)full_size_x/(double)full_size_y;

    haveImage = true;

    // I basically do this function manually, so I can account for 'empty' pixels
//    ImageObject->rescaleDataRange(true);
    ImageObject->setDataRange(QCPRange(minHeight, maxHeight));

    if (reset)
        resetAxes(false);

    cropImage(redraw);
}

void
ImagePlotWidget::SetImagePlot(const std::vector<double> &image, bool redraw) {
    // simple check that all our data is compatible
    if (full_size_x*full_size_y != (int)image.size())
        throw std::runtime_error("Attempting to display image with size not matching given dimensions.");

    // clear any old images
    clearImage();

    // create our new object to actually show the data
    ImageObject = new QCPColorMap(xAxis, yAxis);
    ImageObject->setGradient(QCPColorGradient::gpGrayscale); // default
    ImageObject->setInterpolate(false);

    SetImageData(image, true, true);
}