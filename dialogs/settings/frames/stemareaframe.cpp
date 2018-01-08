#include <QtWidgets/QMessageBox>
#include <utilities/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include "stemareaframe.h"
#include "ui_stemareaframe.h"

StemAreaFrame::StemAreaFrame(QWidget *parent, std::shared_ptr<StemArea> stem, std::shared_ptr<SimulationArea> sim) :
    QWidget(parent), stemSimArea(stem), simArea(sim),
    ui(new Ui::StemAreaFrame)
{
    ui->setupUi(this);

    auto xRangeTup = simArea->getLimitsX();
    auto yRangeTup = simArea->getLimitsY();

    ui->lblRangeX->setText(QString::fromStdString( "        x range: " + Utils::numToString( xRangeTup[0] ) + " to " + Utils::numToString( xRangeTup[1] )));
    ui->lblRangeY->setText(QString::fromStdString( "        y range: " + Utils::numToString( yRangeTup[0] ) + " to " + Utils::numToString( yRangeTup[1] )));

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp("[+-]?(\\d*(?:\\.\\d*)?(?:[eE]([+\\-]?\\d+)?)>)*"));
    QRegExpValidator* pIntValidator = new QRegExpValidator(QRegExp("[+]?\\d*"));

    ui->edtStartX->setValidator(pmValidator);
    ui->edtStartY->setValidator(pmValidator);
    ui->edtFinishX->setValidator(pmValidator);
    ui->edtFinishY->setValidator(pmValidator);

    ui->edtPixelsX->setValidator(pIntValidator);
    ui->edtPixelsY->setValidator(pIntValidator);

    connect(ui->edtPixelsX, SIGNAL(textChanged(QString)), this, SLOT(checkEditZero(QString)));
    connect(ui->edtPixelsY, SIGNAL(textChanged(QString)), this, SLOT(checkEditZero(QString)));

    connect(ui->edtStartX, SIGNAL(textChanged(QString)), this, SLOT(checkValidX(QString)));
    connect(ui->edtFinishX, SIGNAL(textChanged(QString)), this, SLOT(checkValidX(QString)));

    connect(ui->edtStartY, SIGNAL(textChanged(QString)), this, SLOT(checkValidY(QString)));
    connect(ui->edtFinishY, SIGNAL(textChanged(QString)), this, SLOT(checkValidY(QString)));

    auto xLims = stemSimArea->getLimitsX();
    auto yLims = stemSimArea->getLimitsY();
    int px = stemSimArea->getPixelsX();
    int py = stemSimArea->getPixelsY();

    ui->edtStartX->setText(QString::fromStdString(Utils::numToString( xLims[0] )));
    ui->edtFinishX->setText(QString::fromStdString(Utils::numToString( xLims[1] )));
    ui->edtPixelsX->setText(QString::fromStdString(Utils::numToString( px )));

    ui->edtStartY->setText(QString::fromStdString(Utils::numToString( yLims[0] )));
    ui->edtFinishY->setText(QString::fromStdString(Utils::numToString( yLims[1] )));
    ui->edtPixelsY->setText(QString::fromStdString(Utils::numToString( py )));

    connect(parent, SIGNAL(okSignal()), this, SLOT(dlgOk_clicked()));
    connect(parent, SIGNAL(cancelSignal()), this, SLOT(dlgCancel_clicked()));
    connect(parent, SIGNAL(applySignal()), this, SLOT(dlgApply_clicked()));
}

StemAreaFrame::~StemAreaFrame()
{
    delete ui;
}

// reset limits to the simulaition area
void StemAreaFrame::on_btnReset_clicked()
{
    auto xRangeTup = simArea->getLimitsX();
    auto yRangeTup = simArea->getLimitsY();

    ui->edtStartX->setText(QString::fromStdString(Utils::numToString( xRangeTup[0] )));
    ui->edtFinishX->setText(QString::fromStdString(Utils::numToString( xRangeTup[1] )));

    ui->edtStartY->setText(QString::fromStdString(Utils::numToString( yRangeTup[0] )));
    ui->edtFinishY->setText(QString::fromStdString(Utils::numToString( yRangeTup[1] )));
}

void StemAreaFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    StemAreaDialog* dlg = dynamic_cast<StemAreaDialog*>(parentWidget());
    dlg->reject();
}

void StemAreaFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    if(dlgApply_clicked())
    {
        StemAreaDialog* dlg = dynamic_cast<StemAreaDialog*>(parentWidget());
        dlg->accept();
    }
}

bool StemAreaFrame::dlgApply_clicked()
{
    float xs = ui->edtStartX->text().toFloat();
    float xf = ui->edtFinishX->text().toFloat();
    float ys = ui->edtStartY->text().toFloat();
    float yf = ui->edtFinishY->text().toFloat();

    int xp = ui->edtPixelsX->text().toInt();
    int yp = ui->edtPixelsY->text().toInt();

    std::string errors("Warning:");
    bool valid = true;

    if (xs >= xf || ys >= yf)
    {
        errors += "\nStarts must be before finishes.";
        valid = false;
    }

    if (xp < 1 || yp < 1)
    {
        errors += "\nPixels must be greater than zero.";
        valid = false;
    }

    auto xRangeTup = simArea->getLimitsX();
    auto yRangeTup = simArea->getLimitsY();

    if(xs < xRangeTup[0] || xf > xRangeTup[1] || ys < yRangeTup[0] || yf > yRangeTup[1])
    {
        errors += "\nSTEM area must be within simulation area.";
        valid = false;
    }

    if(!valid)
    {
        // need a warning dialog here, as having didgy values will just fuck up loads of calculations
        QMessageBox::warning(this, tr("Invalid range"), tr(errors.c_str()), QMessageBox::Ok);
        return false;
    }

    stemSimArea->forcePxRangeX(xs, xf, xp);
    stemSimArea->forcePxRangeY(ys, yf, yp);
    stemSimArea->setFixed(ui->chkLock->isChecked());

    emit areaChanged();

    return true;
}

void StemAreaFrame::checkEditZero(QString dud)
{
    auto * edt = qobject_cast<QLineEdit*>(sender());

    if(edt == nullptr)
        return;

    float val = edt->text().toInt();

    if (val <= 0)
        edt->setStyleSheet("color: #FF8C00"); // I just chose orange, mgiht want to be a better colour
    else
        edt->setStyleSheet("");
}

void StemAreaFrame::checkValidX(QString dud)
{
    auto xRangeTup = simArea->getLimitsX();
    checkRangeValid(ui->edtStartX, ui->edtFinishX, xRangeTup[0], xRangeTup[1]);
}

void StemAreaFrame::checkValidY(QString dud)
{
    auto yRangeTup = simArea->getLimitsY();
    checkRangeValid(ui->edtStartY, ui->edtFinishY, yRangeTup[0], yRangeTup[1]);
}

void StemAreaFrame::checkRangeValid(QLineEdit* start, QLineEdit* finish, float simStart, float simFinish)
{
    float s = start->text().toFloat();
    float f = finish->text().toFloat();

    // both are invalid so we just set the colour and return
    if (s >= f)
    {
        start->setStyleSheet("color: #FF8C00");
        finish->setStyleSheet("color: #FF8C00");
        return;
    }

    if (s < simStart)
        start->setStyleSheet("color: #FF8C00");
    else
        start->setStyleSheet("");

    if (f > simFinish)
        finish->setStyleSheet("color: #FF8C00");
    else
        finish->setStyleSheet("");
}
