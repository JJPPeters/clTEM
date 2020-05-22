#include <QtWidgets/QMessageBox>
#include <utilities/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include "stemareaframe.h"
#include "ui_stemareaframe.h"

#include "utilities/logging.h"

StemAreaFrame::StemAreaFrame(QWidget *parent, StemArea sa, std::shared_ptr<CrystalStructure> struc) :
    QWidget(parent), Area(sa), Structure(struc),
    ui(new Ui::StemAreaFrame)
{
    ui->setupUi(this);

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));
    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));
    QRegExpValidator* pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));

    ui->edtStartX->setValidator(pmValidator);
    ui->edtStartY->setValidator(pmValidator);
    ui->edtFinishX->setValidator(pmValidator);
    ui->edtFinishY->setValidator(pmValidator);

    ui->edtRangeX->setValidator(pValidator);
    ui->edtRangeY->setValidator(pValidator);

    ui->edtPixelsX->setValidator(pIntValidator);
    ui->edtPixelsY->setValidator(pIntValidator);

    ui->edtPadding->setValidator(pValidator);

    ui->edtStartX->setUnits("Å");
    ui->edtStartY->setUnits("Å");
    ui->edtFinishX->setUnits("Å");
    ui->edtFinishY->setUnits("Å");

    ui->edtRangeX->setUnits("Å");
    ui->edtRangeY->setUnits("Å");

    ui->edtPadding->setUnits("Å");

    connect(ui->edtPixelsX, &QLineEdit::textChanged, this, &StemAreaFrame::valuesChanged);
    connect(ui->edtPixelsY, &QLineEdit::textChanged, this, &StemAreaFrame::valuesChanged);

    connect(ui->edtStartX, &QLineEdit::textChanged, this, &StemAreaFrame::xStartRangeChanged);
    connect(ui->edtFinishX, &QLineEdit::textChanged, this, &StemAreaFrame::xFinishChanged);
    connect(ui->edtRangeX, &QLineEdit::textChanged, this, &StemAreaFrame::xStartRangeChanged);

    connect(ui->edtStartY, &QLineEdit::textChanged, this, &StemAreaFrame::yStartRangeChanged);
    connect(ui->edtFinishY, &QLineEdit::textChanged, this, &StemAreaFrame::yFinishChanged);
    connect(ui->edtRangeY, &QLineEdit::textChanged, this, &StemAreaFrame::yStartRangeChanged);

    connect(ui->edtPadding, &QLineEdit::textChanged, this, &StemAreaFrame::valuesChanged);

    connect(ui->edtStartX, &QLineEdit::editingFinished, this, &StemAreaFrame::editing_finished);
    connect(ui->edtStartY, &QLineEdit::editingFinished, this, &StemAreaFrame::editing_finished);
    connect(ui->edtFinishX, &QLineEdit::editingFinished, this, &StemAreaFrame::editing_finished);
    connect(ui->edtFinishY, &QLineEdit::editingFinished, this, &StemAreaFrame::editing_finished);
    connect(ui->edtPixelsX, &QLineEdit::editingFinished, this, &StemAreaFrame::editing_finished);
    connect(ui->edtPixelsY, &QLineEdit::editingFinished, this, &StemAreaFrame::editing_finished);
    connect(ui->edtRangeX, &QLineEdit::editingFinished, this, &StemAreaFrame::editing_finished);
    connect(ui->edtRangeY, &QLineEdit::editingFinished, this, &StemAreaFrame::editing_finished);
    connect(ui->edtPadding, &QLineEdit::editingFinished, this, &StemAreaFrame::editing_finished);

    // set the default values
    on_btnReset_clicked();
}

StemAreaFrame::~StemAreaFrame()
{
    delete ui;
}

void StemAreaFrame::valuesChanged(QString dud) {
    // group this up, maybe not the best idea
    if (ui->edtPixelsX->text().toInt() > 0)
        ui->edtPixelsX->setStyleSheet("");
    else
        ui->edtPixelsX->setStyleSheet("color: #FF8C00");

    if (ui->edtPixelsY->text().toInt() > 0)
        ui->edtPixelsY->setStyleSheet("");
    else
        ui->edtPixelsY->setStyleSheet("color: #FF8C00");

    // simply emit our signal
    emit areaChanged();
}

void StemAreaFrame::xStartRangeChanged(QString dud) {
    auto range = ui->edtRangeX->text().toDouble();
    auto finish_x = ui->edtStartX->text().toDouble() + range;

    if (!ui->edtFinishX->hasFocus())
        ui->edtFinishX->setText(Utils_Qt::numToQString( finish_x ));
    setInvalidXWarning(checkValidXValues());
    emit areaChanged();
}

void StemAreaFrame::yStartRangeChanged(QString dud) {
    auto range = ui->edtRangeY->text().toDouble();
    auto finish_y = ui->edtStartY->text().toDouble() + range;

    if (!ui->edtFinishY->hasFocus())
        ui->edtFinishY->setText(Utils_Qt::numToQString( finish_y ));
    setInvalidYWarning(checkValidYValues());
    emit areaChanged();
}

void StemAreaFrame::xFinishChanged(QString dud) {
    auto range =  ui->edtFinishX->text().toDouble() - ui->edtStartX->text().toDouble();
    if (!ui->edtRangeX->hasFocus())
        ui->edtRangeX->setText(Utils_Qt::numToQString( range ));
    auto finish = ui->edtStartX->text().toDouble() + range;
    if (!ui->edtFinishX->hasFocus())
        ui->edtFinishX->setText(Utils_Qt::numToQString( finish ));

    // check that values are correct
    setInvalidXWarning(checkValidXValues());
    emit areaChanged();
}

void StemAreaFrame::yFinishChanged(QString dud) {
    auto range =  ui->edtFinishY->text().toDouble() - ui->edtStartY->text().toDouble();
    if (!ui->edtRangeY->hasFocus())
        ui->edtRangeY->setText(Utils_Qt::numToQString( range ));
    auto finish = ui->edtStartY->text().toDouble() + range;
    if (!ui->edtFinishY->hasFocus())
        ui->edtFinishY->setText(Utils_Qt::numToQString( finish ));

    // check that values are correct
    setInvalidYWarning(checkValidYValues());
    emit areaChanged();
}

void StemAreaFrame::setInvalidXWarning(bool valid) {
    if (valid) {
        ui->edtStartX->setStyleSheet("");
        ui->edtFinishX->setStyleSheet("");
        ui->edtRangeX->setStyleSheet("");
    } else {
        ui->edtStartX->setStyleSheet("color: #FF8C00");
        ui->edtFinishX->setStyleSheet("color: #FF8C00");
        ui->edtRangeX->setStyleSheet("color: #FF8C00");
    }
}

void StemAreaFrame::setInvalidYWarning(bool valid) {
    if (valid) {
        ui->edtStartY->setStyleSheet("");
        ui->edtFinishY->setStyleSheet("");
        ui->edtRangeY->setStyleSheet("");
    } else {
        ui->edtStartY->setStyleSheet("color: #FF8C00");
        ui->edtFinishY->setStyleSheet("color: #FF8C00");
        ui->edtRangeY->setStyleSheet("color: #FF8C00");
    }
}

bool StemAreaFrame::checkValidXValues()  { return ui->edtRangeX->text().toDouble() > 0.0f; }
bool StemAreaFrame::checkValidYValues()  { return ui->edtRangeY->text().toDouble() > 0.0f; }

StemArea StemAreaFrame::stemArea() {
    if (!checkValidXValues() || !checkValidYValues())
        return Area;

    auto xs = ui->edtStartX->text().toDouble();
    auto ys = ui->edtStartY->text().toDouble();
    auto xf = ui->edtFinishX->text().toDouble();
    auto yf = ui->edtFinishY->text().toDouble();

    auto xp = ui->edtPixelsX->text().toInt();
    auto yp = ui->edtPixelsY->text().toInt();

    auto pd = ui->edtPadding->text().toDouble();

    return StemArea(xs, xf, ys, yf, xp, yp, pd);
}

void StemAreaFrame::on_btnReset_clicked() {
    auto xLims = Area.getRawLimitsX();
    auto yLims = Area.getRawLimitsY();
    int px = Area.getPixelsX();
    int py = Area.getPixelsY();
    double padding = Area.getPadding();

    ui->edtStartX->setText(Utils_Qt::numToQString( xLims[0] ));
    ui->edtFinishX->setText(Utils_Qt::numToQString( xLims[1] ));
    ui->edtPixelsX->setText(Utils_Qt::numToQString( px ));
    ui->edtRangeX->setText(Utils_Qt::numToQString( xLims[1] - xLims[0] ));

    ui->edtStartY->setText(Utils_Qt::numToQString( yLims[0] ));
    ui->edtFinishY->setText(Utils_Qt::numToQString( yLims[1] ));
    ui->edtPixelsY->setText(Utils_Qt::numToQString( py ));
    ui->edtRangeY->setText(Utils_Qt::numToQString( yLims[1] - yLims[0] ));

    ui->edtPadding->setText(Utils_Qt::numToQString(padding));

    emit areaChanged();
}

void StemAreaFrame::on_btnDefault_clicked() {
    // TODO: set to structure limits
    // TODO: Pixels are defined in the json file -> get them here somehow?
    if (Structure) {
        auto xLims = Structure->getLimitsX();
        auto yLims = Structure->getLimitsY();
        double padding = 0.f;
        ui->edtStartX->setText(Utils_Qt::numToQString(xLims[0]));
        ui->edtFinishX->setText(Utils_Qt::numToQString(xLims[1]));
//        ui->edtPixelsX->setText(Utils_Qt::numToQString( px ));
        ui->edtRangeX->setText(Utils_Qt::numToQString(xLims[1] - xLims[0]));

        ui->edtStartY->setText(Utils_Qt::numToQString(yLims[0]));
        ui->edtFinishY->setText(Utils_Qt::numToQString(yLims[1]));
//        ui->edtPixelsY->setText(Utils_Qt::numToQString( py ));
        ui->edtRangeY->setText(Utils_Qt::numToQString(yLims[1] - yLims[0]));

        ui->edtPadding->setText(Utils_Qt::numToQString(padding));
    } else {
        on_btnReset_clicked();
    }

    emit areaChanged();
}

void StemAreaFrame::editing_finished() {
    QLineEdit* sndr = (QLineEdit*) sender();

    auto val = sndr->text().toDouble();
    sndr->setText(Utils_Qt::numToQString( val ));
}