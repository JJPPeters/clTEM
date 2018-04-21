#include <utils/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include <QtGui/QRegExpValidator>
#include "ctemareaframe.h"
#include "ui_ctemareaframe.h"

//TODO: Hover over should give original values?

CtemAreaFrame::CtemAreaFrame(QWidget *parent, SimulationArea sa) :
    QWidget(parent), simArea(sa),
    ui(new Ui::CtemAreaFrame)
{
    ui->setupUi(this);

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp("[+-]?(\\d*(?:\\.\\d*)?(?:[eE]([+\\-]?\\d+)?)>)*"));
    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp("[+]?(\\d*(?:\\.\\d*)?(?:[eE]([+\\-]?\\d+)?)>)*"));

    ui->edtStartX->setValidator(pmValidator);
    ui->edtStartY->setValidator(pmValidator);
    ui->edtFinishX->setValidator(pmValidator);
    ui->edtFinishY->setValidator(pmValidator);

    ui->edtRangeXY->setValidator(pValidator);

    ui->edtStartX->setUnits("Å");
    ui->edtStartY->setUnits("Å");
    ui->edtFinishX->setUnits("Å");
    ui->edtFinishY->setUnits("Å");
    ui->edtRangeXY->setUnits("Å");

    // set teh default values
    on_btnReset_clicked();

    connect(ui->edtStartX, SIGNAL(textChanged(QString)), this, SLOT(rangeChanged(QString)));
    connect(ui->edtStartY, SIGNAL(textChanged(QString)), this, SLOT(rangeChanged(QString)));

    connect(ui->edtFinishX, SIGNAL(textChanged(QString)), this, SLOT(xFinishChanged(QString)));
    connect(ui->edtFinishY, SIGNAL(textChanged(QString)), this, SLOT(yFinishChanged(QString)));

    connect(ui->edtRangeXY, SIGNAL(textChanged(QString)), this, SLOT(rangeChanged(QString)));

    connect(ui->edtStartX, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtStartY, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtFinishX, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtFinishY, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
    connect(ui->edtRangeXY, SIGNAL(editingFinished()), this, SLOT(editing_finished()));
}

CtemAreaFrame::~CtemAreaFrame()
{
    delete ui;
}

void CtemAreaFrame::xFinishChanged(QString dud) {
    auto range =  ui->edtFinishX->text().toFloat() - ui->edtStartX->text().toFloat();
    if (!ui->edtRangeXY->hasFocus())
        ui->edtRangeXY->setText(Utils_Qt::numToQString( range ));

    auto finish = ui->edtStartY->text().toFloat() + range;
    if (!ui->edtFinishY->hasFocus())
        ui->edtFinishY->setText(Utils_Qt::numToQString( finish ));

    // check that values are correct
    setInvalidWarning(checkValidValues());
    emit areaChanged();
}

void CtemAreaFrame::yFinishChanged(QString dud) {
    auto range =  ui->edtFinishY->text().toFloat() - ui->edtStartY->text().toFloat();
    if (!ui->edtRangeXY->hasFocus())
        ui->edtRangeXY->setText(Utils_Qt::numToQString( range ));
    auto finish = ui->edtStartX->text().toFloat() + range;
    if (!ui->edtFinishX->hasFocus())
        ui->edtFinishX->setText(Utils_Qt::numToQString( finish ));

    // check that values are correct
    setInvalidWarning(checkValidValues());
    emit areaChanged();
}

void CtemAreaFrame::rangeChanged(QString dud) {
    auto range = ui->edtRangeXY->text().toFloat();
    auto finish_x = ui->edtStartX->text().toFloat() + range;
    auto finish_y = ui->edtStartY->text().toFloat() + range;

    if (!ui->edtFinishX->hasFocus())
        ui->edtFinishX->setText(Utils_Qt::numToQString( finish_x ));
    if (!ui->edtFinishY->hasFocus())
        ui->edtFinishY->setText(Utils_Qt::numToQString( finish_y ));

    // check that values are correct
    setInvalidWarning(checkValidValues());
    emit areaChanged();
}

bool CtemAreaFrame::checkValidValues()
{
    // check range
    return ui->edtRangeXY->text().toFloat() > 0.0f;
}

void CtemAreaFrame::setInvalidWarning(bool valid) {
    if (valid) {
        ui->edtStartX->setStyleSheet("");
        ui->edtStartY->setStyleSheet("");
        ui->edtFinishX->setStyleSheet("");
        ui->edtFinishY->setStyleSheet("");
        ui->edtRangeXY->setStyleSheet("");
    } else {
        ui->edtStartX->setStyleSheet("color: #FF8C00");
        ui->edtStartY->setStyleSheet("color: #FF8C00");
        ui->edtFinishX->setStyleSheet("color: #FF8C00");
        ui->edtFinishY->setStyleSheet("color: #FF8C00");
        ui->edtRangeXY->setStyleSheet("color: #FF8C00");
    }
}

SimulationArea CtemAreaFrame::getSimArea() {
    if (!checkValidValues())
        return simArea;

    auto xs = ui->edtStartX->text().toFloat();
    auto ys = ui->edtStartY->text().toFloat();
    auto xf = ui->edtFinishX->text().toFloat();
    auto yf = ui->edtFinishY->text().toFloat();

    return SimulationArea(xs, xf, ys, yf);
}

void CtemAreaFrame::on_btnReset_clicked() {
    auto xRangeTup = simArea.getLimitsX();
    auto yRangeTup = simArea.getLimitsY();

    ui->edtStartX->setText(Utils_Qt::numToQString( xRangeTup[0] ));
    ui->edtFinishX->setText(Utils_Qt::numToQString( xRangeTup[1] ));

    ui->edtStartY->setText(Utils_Qt::numToQString( yRangeTup[0] ));
    ui->edtFinishY->setText(Utils_Qt::numToQString( yRangeTup[1] ));

    auto range = xRangeTup[1] - xRangeTup[0]; // this will be the same as the y one

    ui->edtRangeXY->setText(Utils_Qt::numToQString( range ));

    emit areaChanged();
}

void CtemAreaFrame::editing_finished() {
    QLineEdit* sndr = (QLineEdit*) sender();

    auto val = sndr->text().toFloat();
    sndr->setText(Utils_Qt::numToQString( val ));
}
