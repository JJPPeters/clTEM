#include <QtWidgets/QMessageBox>
#include <utilities/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include "stemareaframe.h"
#include "ui_stemareaframe.h"

StemAreaFrame::StemAreaFrame(QWidget *parent, StemArea sa) :
    QWidget(parent), Area(sa),
    ui(new Ui::StemAreaFrame)
{
    ui->setupUi(this);

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp("[+-]?(\\d*(?:\\.\\d*)?(?:[eE]([+\\-]?\\d+)?)>)*"));
    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp("[+]?(\\d*(?:\\.\\d*)?(?:[eE]([+\\-]?\\d+)?)>)*"));
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

    connect(ui->edtPixelsX, SIGNAL(textChanged(QString)), this, SLOT(valuesChanged(QString)));
    connect(ui->edtPixelsY, SIGNAL(textChanged(QString)), this, SLOT(valuesChanged(QString)));

    connect(ui->edtStartX, SIGNAL(textChanged(QString)), this, SLOT(xStartRangeChanged(QString)));
    connect(ui->edtFinishX, SIGNAL(textChanged(QString)), this, SLOT(xFinishChanged(QString)));
    connect(ui->edtRangeX, SIGNAL(textChanged(QString)), this, SLOT(xStartRangeChanged(QString)));

    connect(ui->edtStartY, SIGNAL(textChanged(QString)), this, SLOT(yStartRangeChanged(QString)));
    connect(ui->edtFinishY, SIGNAL(textChanged(QString)), this, SLOT(yFinishChanged(QString)));
    connect(ui->edtRangeY, SIGNAL(textChanged(QString)), this, SLOT(yStartRangeChanged(QString)));

    connect(ui->edtPadding, SIGNAL(textChanged(QString)), this, SLOT(valuesChanged(QString)));

    connect(ui->edtStartX, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtStartY, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtFinishX, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtFinishY, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtPixelsX, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtPixelsY, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtRangeX, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtRangeY, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtPadding, SIGNAL(editingFinished()), this, SLOT(editing_finished()));


    // set teh default values
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
    auto range = ui->edtRangeX->text().toFloat();
    auto finish_x = ui->edtStartX->text().toFloat() + range;

    if (!ui->edtFinishX->hasFocus())
        ui->edtFinishX->setText(Utils::numToQString( finish_x, edt_precision ));
    setInvalidXWarning(checkValidXValues());
    emit areaChanged();
}

void StemAreaFrame::yStartRangeChanged(QString dud) {
    auto range = ui->edtRangeY->text().toFloat();
    auto finish_y = ui->edtStartY->text().toFloat() + range;

    if (!ui->edtFinishY->hasFocus())
        ui->edtFinishY->setText(Utils::numToQString( finish_y, edt_precision ));
    setInvalidYWarning(checkValidYValues());
    emit areaChanged();
}

void StemAreaFrame::xFinishChanged(QString dud) {
    auto range =  ui->edtFinishX->text().toFloat() - ui->edtStartX->text().toFloat();
    if (!ui->edtRangeX->hasFocus())
        ui->edtRangeX->setText(Utils::numToQString( range, edt_precision ));
    auto finish = ui->edtStartX->text().toFloat() + range;
    if (!ui->edtFinishX->hasFocus())
        ui->edtFinishX->setText(Utils::numToQString( finish, edt_precision ));

    // check that values are correct
    setInvalidXWarning(checkValidXValues());
    emit areaChanged();
}

void StemAreaFrame::yFinishChanged(QString dud) {
    auto range =  ui->edtFinishY->text().toFloat() - ui->edtStartY->text().toFloat();
    if (!ui->edtRangeY->hasFocus())
        ui->edtRangeY->setText(Utils::numToQString( range, edt_precision ));
    auto finish = ui->edtStartY->text().toFloat() + range;
    if (!ui->edtFinishY->hasFocus())
        ui->edtFinishY->setText(Utils::numToQString( finish, edt_precision ));

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

bool StemAreaFrame::checkValidXValues()  { return ui->edtRangeX->text().toFloat() > 0.0f; }
bool StemAreaFrame::checkValidYValues()  { return ui->edtRangeY->text().toFloat() > 0.0f; }

StemArea StemAreaFrame::getStemArea() {
    if (!checkValidXValues() || !checkValidYValues())
        return Area;

    auto xs = ui->edtStartX->text().toFloat();
    auto ys = ui->edtStartY->text().toFloat();
    auto xf = ui->edtFinishX->text().toFloat();
    auto yf = ui->edtFinishY->text().toFloat();

    auto xp = ui->edtPixelsX->text().toInt();
    auto yp = ui->edtPixelsY->text().toInt();

    auto pd = ui->edtPadding->text().toFloat();

    return StemArea(xs, xf, ys, yf, xp, yp, pd);
}

void StemAreaFrame::on_btnReset_clicked() {
    auto xLims = Area.getLimitsX();
    auto yLims = Area.getLimitsY();
    int px = Area.getPixelsX();
    int py = Area.getPixelsY();
    float padding = Area.getPadding();

    ui->edtStartX->setText(Utils::numToQString( xLims[0], edt_precision ));
    ui->edtFinishX->setText(Utils::numToQString( xLims[1], edt_precision ));
    ui->edtPixelsX->setText(Utils::numToQString( px, edt_precision ));
    ui->edtRangeX->setText(Utils::numToQString( xLims[1] - xLims[0], edt_precision ));

    ui->edtStartY->setText(Utils::numToQString( yLims[0], edt_precision ));
    ui->edtFinishY->setText(Utils::numToQString( yLims[1], edt_precision ));
    ui->edtPixelsY->setText(Utils::numToQString( py, edt_precision ));
    ui->edtRangeY->setText(Utils::numToQString( yLims[1] - yLims[0], edt_precision ));

    ui->edtPadding->setText(Utils::numToQString(padding, edt_precision));

    emit areaChanged();
}

void StemAreaFrame::editing_finished() {
    QLineEdit* sndr = (QLineEdit*) sender();

    auto val = sndr->text().toFloat();
    sndr->setText(Utils::numToQString( val, edt_precision ));
}