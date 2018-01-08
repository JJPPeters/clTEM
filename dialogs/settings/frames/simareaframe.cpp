#include <utilities/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include "simareaframe.h"
#include "ui_simareaframe.h"

//TODO: Hover over should give original values?

SimAreaFrame::SimAreaFrame(QWidget *parent, std::shared_ptr<SimulationArea> sa, std::shared_ptr<CrystalStructure> struc) :
    QWidget(parent), simArea(sa), structure(struc), ui(new Ui::SimAreaFrame)
{
    ui->setupUi(this);

    if (struc)
    {
//        xRange = std::get<0>(struc->getStructRanges());
//        yRange = std::get<1>(struc->getStructRanges());
//        ui->lblRangeX->setText(QString::fromStdString( "        x range: 0 to " + Utils::numToString( xRange )));
//        ui->lblRangeY->setText(QString::fromStdString( "        y range: 0 to " + Utils::numToString( yRange )));
        auto xr = structure->getLimitsX();
        auto yr = structure->getLimitsY();
        ui->lblRangeX->setText(QString::fromStdString( "        x range: " + Utils::numToString( xr[0] ) + " to " + Utils::numToString( xr[1] )));
        ui->lblRangeY->setText(QString::fromStdString( "        y range: " + Utils::numToString( yr[0] ) + " to " + Utils::numToString( yr[1] )));
    }
    else
    {
        ui->lblRangeX->setText(QString::fromStdString("        x range: -- to --"));
        ui->lblRangeY->setText(QString::fromStdString("        y range: -- to --"));
    }

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp("[+-]?(\\d*(?:\\.\\d*)?(?:[eE]([+\\-]?\\d+)?)>)*"));

    ui->edtStartX->setValidator(pmValidator);
    ui->edtStartY->setValidator(pmValidator);
    ui->edtFinishX->setValidator(pmValidator);
    ui->edtFinishY->setValidator(pmValidator);

    connect(ui->edtStartX, SIGNAL(textChanged(QString)), this, SLOT(checkEditZeroX(QString)));
    connect(ui->edtFinishX, SIGNAL(textChanged(QString)), this, SLOT(checkEditZeroX(QString)));
    connect(ui->edtStartY, SIGNAL(textChanged(QString)), this, SLOT(checkEditZeroY(QString)));
    connect(ui->edtFinishY, SIGNAL(textChanged(QString)), this, SLOT(checkEditZeroY(QString)));

    auto xRangeTup = simArea->getLimitsX();
    auto yRangeTup = simArea->getLimitsY();

    ui->edtStartX->setText(QString::fromStdString(Utils::numToString( xRangeTup[0] )));
    ui->edtFinishX->setText(QString::fromStdString(Utils::numToString( xRangeTup[1] )));

    ui->edtStartY->setText(QString::fromStdString(Utils::numToString( yRangeTup[0] )));
    ui->edtFinishY->setText(QString::fromStdString(Utils::numToString( yRangeTup[1] )));

    ui->chkLock->setChecked(simArea->getIsFixed());

    connect(parent, SIGNAL(okSignal()), this, SLOT(dlgOk_clicked()));
    connect(parent, SIGNAL(cancelSignal()), this, SLOT(dlgCancel_clicked()));
    connect(parent, SIGNAL(applySignal()), this, SLOT(dlgApply_clicked()));
}

SimAreaFrame::~SimAreaFrame()
{
    delete ui;
}

void SimAreaFrame::on_btnReset_clicked()
{
    auto xr = structure->getLimitsX();
    auto yr = structure->getLimitsY();

    ui->edtStartX->setText(QString::fromStdString(Utils::numToString(xr[0])));
    ui->edtStartY->setText(QString::fromStdString(Utils::numToString(yr[0])));

    ui->edtFinishX->setText(QString::fromStdString(Utils::numToString(xr[1])));
    ui->edtFinishY->setText(QString::fromStdString(Utils::numToString(yr[1])));
//    ui->edtStartX->setText("0");
//    ui->edtStartY->setText("0");
//
//    ui->edtFinishX->setText(QString::fromStdString(Utils::numToString(xRange)));
//    ui->edtFinishY->setText(QString::fromStdString(Utils::numToString(yRange)));
}

void SimAreaFrame::checkEditZeroX(QString dud) {checkEditZero(ui->edtStartX, ui->edtFinishX);}
void SimAreaFrame::checkEditZeroY(QString dud) {checkEditZero(ui->edtStartY, ui->edtFinishY);}
void SimAreaFrame::checkEditZero(QLineEdit* edtStart, QLineEdit* edtFinish)
{
    if (edtStart->text().toFloat() >= edtFinish->text().toFloat())
    {
        edtStart->setStyleSheet("color: #FF8C00");
        edtFinish->setStyleSheet("color: #FF8C00");
    }
    else
    {
        edtStart->setStyleSheet("");
        edtFinish->setStyleSheet("");
    }
}

void SimAreaFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    SimAreaDialog* dlg = static_cast<SimAreaDialog*>(parentWidget());
    dlg->reject();
}

void SimAreaFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    if(dlgApply_clicked())
    {
        SimAreaDialog* dlg = static_cast<SimAreaDialog*>(parentWidget());
        dlg->accept();
    }
}

bool SimAreaFrame::dlgApply_clicked()
{
    float xs = ui->edtStartX->text().toFloat();
    float xf = ui->edtFinishX->text().toFloat();
    float ys = ui->edtStartY->text().toFloat();
    float yf = ui->edtFinishY->text().toFloat();

    if (xs >= xf || ys >= yf)
    {
        // need a warning dialog here, as having didgy values will just fuck up loads of calculations
        QMessageBox::warning(this, tr("Invalid range"), tr("Warning:\nStart must be before finish!"), QMessageBox::Ok);
        return false;
    }

    simArea->setRangeX(xs, xf);
    simArea->setRangeY(ys, yf);
    simArea->setFixed(ui->chkLock->isChecked());

    emit areaChanged();

    return true;
}
