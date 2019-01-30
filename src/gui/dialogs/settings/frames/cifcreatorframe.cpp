//
// Created by Jon on 30/01/2019.
//

#include <utils/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include "cifcreatorframe.h"
#include "ui_cifcreatorframe.h"

CifCreatorFrame::CifCreatorFrame(QWidget *parent, CIF::CIFReader _cif, std::shared_ptr<CIF::SuperCellInfo> _info) : QWidget(parent), ui(new Ui::CifCreatorFrame), CellInfo(_info), cif(_cif) {
    ui->setupUi(this);

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));
    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtRangeX->setValidator(pValidator);
    ui->edtRangeY->setValidator(pValidator);
    ui->edtRangeZ->setValidator(pValidator);

    ui->edtZoneU->setValidator(pmValidator);
    ui->edtZoneV->setValidator(pmValidator);
    ui->edtZoneW->setValidator(pmValidator);

    ui->edtZoneA->setValidator(pmValidator);
    ui->edtZoneB->setValidator(pmValidator);
    ui->edtZoneC->setValidator(pmValidator);

    ui->edtTiltA->setValidator(pmValidator);
    ui->edtTiltB->setValidator(pmValidator);
    ui->edtTiltC->setValidator(pmValidator);

    ui->edtRangeX->setUnits("Å");
    ui->edtRangeY->setUnits("Å");
    ui->edtRangeZ->setUnits("Å");

    ui->edtTiltA->setUnits("°");
    ui->edtTiltB->setUnits("°");
    ui->edtTiltC->setUnits("°");

    Eigen::Vector3d uvw = CellInfo->uvw;
    ui->edtZoneU->setText(Utils_Qt::numToQString( uvw(0) ));
    ui->edtZoneV->setText(Utils_Qt::numToQString( uvw(1) ));
    ui->edtZoneW->setText(Utils_Qt::numToQString( uvw(2) ));

    Eigen::Vector3d abc = CellInfo->abc;
    ui->edtZoneA->setText(Utils_Qt::numToQString( abc(0) ));
    ui->edtZoneB->setText(Utils_Qt::numToQString( abc(1) ));
    ui->edtZoneC->setText(Utils_Qt::numToQString( abc(2) ));

    Eigen::Vector3d range = CellInfo->widths;
    ui->edtRangeX->setText(Utils_Qt::numToQString( range(0) ));
    ui->edtRangeY->setText(Utils_Qt::numToQString( range(1) ));
    ui->edtRangeZ->setText(Utils_Qt::numToQString( range(2) ));

    Eigen::Vector3d tilts = CellInfo->tilts;
    ui->edtTiltA->setText(Utils_Qt::numToQString( tilts(0) ));
    ui->edtTiltB->setText(Utils_Qt::numToQString( tilts(1) ));
    ui->edtTiltC->setText(Utils_Qt::numToQString( tilts(2) ));

    auto parent_dlg = dynamic_cast<CifCreatorDialog*>(parentWidget());
    connect(parent_dlg, &CifCreatorDialog::okSignal, this, &CifCreatorFrame::dlgOk_clicked);
    connect(parent_dlg, &CifCreatorDialog::cancelSignal, this, &CifCreatorFrame::dlgCancel_clicked);
    connect(parent_dlg, &CifCreatorDialog::applySignal, this, &CifCreatorFrame::dlgApply_clicked);

    connect(ui->edtRangeX, &QLineEdit::textChanged, this, &CifCreatorFrame::rangeValuesChanged);
    connect(ui->edtRangeY, &QLineEdit::textChanged, this, &CifCreatorFrame::rangeValuesChanged);
    connect(ui->edtRangeZ, &QLineEdit::textChanged, this, &CifCreatorFrame::rangeValuesChanged);

    connect(ui->edtZoneU, &QLineEdit::textChanged, this, &CifCreatorFrame::directionValuesChanged);
    connect(ui->edtZoneV, &QLineEdit::textChanged, this, &CifCreatorFrame::directionValuesChanged);
    connect(ui->edtZoneW, &QLineEdit::textChanged, this, &CifCreatorFrame::directionValuesChanged);

    connect(ui->edtZoneA, &QLineEdit::textChanged, this, &CifCreatorFrame::directionValuesChanged);
    connect(ui->edtZoneB, &QLineEdit::textChanged, this, &CifCreatorFrame::directionValuesChanged);
    connect(ui->edtZoneC, &QLineEdit::textChanged, this, &CifCreatorFrame::directionValuesChanged);
}

void CifCreatorFrame::directionValuesChanged(QString dud) {
    double u = ui->edtZoneU->text().toDouble();
    double v = ui->edtZoneV->text().toDouble();
    double w = ui->edtZoneW->text().toDouble();
    Eigen::Vector3d uvw;
    uvw << u, v, w;

    double a = ui->edtZoneA->text().toDouble();
    double b = ui->edtZoneB->text().toDouble();
    double c = ui->edtZoneC->text().toDouble();
    Eigen::Vector3d abc;
    abc << a, b, c;

    uvw.normalize();
    abc.normalize();

    if(uvw != abc) { // I think this check is fine?
        ui->edtZoneU->setStyleSheet("");
        ui->edtZoneV->setStyleSheet("");
        ui->edtZoneW->setStyleSheet("");
        ui->edtZoneA->setStyleSheet("");
        ui->edtZoneB->setStyleSheet("");
        ui->edtZoneC->setStyleSheet("");
    } else {
        ui->edtZoneU->setStyleSheet("color: #FF8C00");
        ui->edtZoneV->setStyleSheet("color: #FF8C00");
        ui->edtZoneW->setStyleSheet("color: #FF8C00");
        ui->edtZoneA->setStyleSheet("color: #FF8C00");
        ui->edtZoneB->setStyleSheet("color: #FF8C00");
        ui->edtZoneC->setStyleSheet("color: #FF8C00");
        return;
    }

    if (u != 0 || v != 0 || w != 0) {
        ui->edtZoneU->setStyleSheet("");
        ui->edtZoneV->setStyleSheet("");
        ui->edtZoneW->setStyleSheet("");
    } else {
        ui->edtZoneU->setStyleSheet("");
        ui->edtZoneV->setStyleSheet("");
        ui->edtZoneW->setStyleSheet("");
        ui->edtZoneU->setStyleSheet("color: #FF8C00");
        ui->edtZoneV->setStyleSheet("color: #FF8C00");
        ui->edtZoneW->setStyleSheet("color: #FF8C00");
    }
}

void CifCreatorFrame::rangeValuesChanged(QString dud) {

    if (ui->edtRangeX->text().toDouble() != 0)
        ui->edtRangeX->setStyleSheet("");
    else
        ui->edtRangeX->setStyleSheet("color: #FF8C00");

    if (ui->edtRangeY->text().toDouble() != 0)
        ui->edtRangeY->setStyleSheet("");
    else
        ui->edtRangeY->setStyleSheet("color: #FF8C00");

    if (ui->edtRangeY->text().toDouble() != 0)
        ui->edtRangeY->setStyleSheet("");
    else
        ui->edtRangeY->setStyleSheet("color: #FF8C00");
}

void CifCreatorFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    auto * dlg = dynamic_cast<CifCreatorDialog*>(parentWidget());
    dlg->reject();
}

void CifCreatorFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    dlgApply_clicked();
    auto * dlg = dynamic_cast<CifCreatorDialog*>(parentWidget());
    dlg->accept();
}

void CifCreatorFrame::dlgApply_clicked()
{
    // Check that uvw is not collinear to abc

    // check widths aren't zero

    // check uvw is not zero

    double xr = ui->edtRangeX->text().toDouble();
    double yr = ui->edtRangeY->text().toDouble();
    double zr = ui->edtRangeZ->text().toDouble();
    CellInfo->setWidths(xr, yr, zr);

    double u = ui->edtZoneU->text().toDouble();
    double v = ui->edtZoneV->text().toDouble();
    double w = ui->edtZoneW->text().toDouble();
    CellInfo->setUVW(u, v, w);

    double a = ui->edtZoneA->text().toDouble();
    double b = ui->edtZoneB->text().toDouble();
    double c = ui->edtZoneC->text().toDouble();
    CellInfo->setABC(a, b, c);

    double ta = ui->edtTiltA->text().toDouble();
    double tb = ui->edtTiltB->text().toDouble();
    double tc = ui->edtTiltC->text().toDouble();
    CellInfo->setABC(ta, tb, tc);
}