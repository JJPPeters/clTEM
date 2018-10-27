#include <utils/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include <QtGui/QRegExpValidator>
#include "ctemareaframe.h"
#include "ui_ctemareaframe.h"

//TODO: Hover over should give original values?

CtemAreaFrame::CtemAreaFrame(QWidget *parent, SimulationArea sa) :
    QWidget(parent), ui(new Ui::CtemAreaFrame), simArea(sa)
{
    ui->setupUi(this);

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));
    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtStartX->setValidator(pmValidator);
    ui->edtStartY->setValidator(pmValidator);
    ui->edtFinishX->setValidator(pmValidator);
    ui->edtFinishY->setValidator(pmValidator);

    ui->edtRangeX->setValidator(pValidator);
    ui->edtRangeY->setValidator(pValidator);

    ui->edtStartX->setUnits("Å");
    ui->edtStartY->setUnits("Å");
    ui->edtFinishX->setUnits("Å");
    ui->edtFinishY->setUnits("Å");
    ui->edtRangeX->setUnits("Å");
    ui->edtRangeY->setUnits("Å");

    // set teh default values
    on_btnReset_clicked();

    connect(ui->edtStartX, &QLineEdit::textChanged, this, &CtemAreaFrame::xRangeChanged);
    connect(ui->edtStartY, &QLineEdit::textChanged, this, &CtemAreaFrame::yRangeChanged);

    connect(ui->edtFinishX, &QLineEdit::textChanged, this, &CtemAreaFrame::xFinishChanged);
    connect(ui->edtFinishY, &QLineEdit::textChanged, this, &CtemAreaFrame::yFinishChanged);

    connect(ui->edtRangeX, &QLineEdit::textChanged, this, &CtemAreaFrame::xRangeChanged);
    connect(ui->edtRangeY, &QLineEdit::textChanged, this, &CtemAreaFrame::yRangeChanged);

    connect(ui->edtStartX, &QLineEdit::editingFinished, this, &CtemAreaFrame::editing_finished);
    connect(ui->edtStartY, &QLineEdit::editingFinished, this, &CtemAreaFrame::editing_finished);
    connect(ui->edtFinishX, &QLineEdit::editingFinished, this, &CtemAreaFrame::editing_finished);
    connect(ui->edtFinishY, &QLineEdit::editingFinished, this, &CtemAreaFrame::editing_finished);
    connect(ui->edtRangeX, &QLineEdit::editingFinished, this, &CtemAreaFrame::editing_finished);
    connect(ui->edtRangeY, &QLineEdit::editingFinished, this, &CtemAreaFrame::editing_finished);
}

CtemAreaFrame::~CtemAreaFrame()
{
    delete ui;
}

void CtemAreaFrame::xFinishChanged(QString dud) {
    auto range =  ui->edtFinishX->text().toFloat() - ui->edtStartX->text().toFloat();
    if (!ui->edtRangeX->hasFocus())
        ui->edtRangeX->setText(Utils_Qt::numToQString( range ));

    // check that values are correct
    setXInvalidWarning(checkXValid());
    emit areaChanged();
}

void CtemAreaFrame::yFinishChanged(QString dud) {
    auto range =  ui->edtFinishY->text().toFloat() - ui->edtStartY->text().toFloat();
    if (!ui->edtRangeY->hasFocus())
        ui->edtRangeY->setText(Utils_Qt::numToQString( range ));

    // check that values are correct
    setYInvalidWarning(checkYValid());
    emit areaChanged();
}

void CtemAreaFrame::xRangeChanged(QString dud) {
    auto range_x = ui->edtRangeX->text().toFloat();
    auto finish_x = ui->edtStartX->text().toFloat() + range_x;

    if (!ui->edtFinishX->hasFocus())
        ui->edtFinishX->setText(Utils_Qt::numToQString( finish_x ));

    // check that values are correct
    setXInvalidWarning(checkXValid());
    emit areaChanged();
}

void CtemAreaFrame::yRangeChanged(QString dud) {
    auto range_y = ui->edtRangeY->text().toFloat();
    auto finish_y = ui->edtStartY->text().toFloat() + range_y;

    if (!ui->edtFinishY->hasFocus())
        ui->edtFinishY->setText(Utils_Qt::numToQString( finish_y ));

    // check that values are correct
    setYInvalidWarning(checkYValid());
    emit areaChanged();
}

bool CtemAreaFrame::checkXValid() {
    // check range
    return ui->edtRangeX->text().toFloat() > 0.0f;
}

bool CtemAreaFrame::checkYValid() {
    // check range
    return ui->edtRangeY->text().toFloat() > 0.0f;
}

void CtemAreaFrame::setXInvalidWarning(bool valid) {
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

void CtemAreaFrame::setYInvalidWarning(bool valid) {
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

SimulationArea CtemAreaFrame::getSimArea() {
    if (!checkXValid() || !checkYValid())
        return simArea;

    auto xs = ui->edtStartX->text().toFloat();
    auto ys = ui->edtStartY->text().toFloat();
    auto xf = ui->edtFinishX->text().toFloat();
    auto yf = ui->edtFinishY->text().toFloat();

    return SimulationArea(xs, xf, ys, yf);
}

void CtemAreaFrame::on_btnReset_clicked() {
    auto xRangeTup = simArea.getRawLimitsX();
    auto yRangeTup = simArea.getRawLimitsY();

    ui->edtStartX->setText(Utils_Qt::numToQString( xRangeTup[0] ));
    ui->edtFinishX->setText(Utils_Qt::numToQString( xRangeTup[1] ));

    ui->edtStartY->setText(Utils_Qt::numToQString( yRangeTup[0] ));
    ui->edtFinishY->setText(Utils_Qt::numToQString( yRangeTup[1] ));

    ui->edtRangeX->setText(Utils_Qt::numToQString( xRangeTup[1] - xRangeTup[0] ));
    ui->edtRangeY->setText(Utils_Qt::numToQString( yRangeTup[1] - yRangeTup[0] ));

    emit areaChanged();
}

void CtemAreaFrame::editing_finished() {
    auto sndr = (QLineEdit*) sender();

    auto val = sndr->text().toFloat();
    sndr->setText(Utils_Qt::numToQString( val ));
}
