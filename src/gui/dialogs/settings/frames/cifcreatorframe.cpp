//
// Created by Jon on 30/01/2019.
//

#include <utils/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include "cifcreatorframe.h"
#include "ui_cifcreatorframe.h"

CifCreatorFrame::CifCreatorFrame(QWidget *parent, CIF::CIFReader _cif, std::shared_ptr<CIF::SuperCellInfo> _info) : QWidget(parent), ui(new Ui::CifCreatorFrame), CellInfo(_info), cif(_cif) {
    ui->setupUi(this);

    // This is not needed, but it might be needed if the computation turns out to be too taxing for some systems
    ui->btnPreview->setVisible(false);
    // Set up the OpenGL stuffs

    try {
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setVersion(4, 0); // sets opengl version

        pltPreview = new OGLViewWidget(this);
        pltPreview->setFormat(format);
        ui->hPlotLayout->addWidget(pltPreview, 1);
        pltPreview->setMinimumHeight(400);
        pltPreview->setMinimumWidth(400);

        pltPreview->setAttribute(Qt::WA_TransparentForMouseEvents);
        connect(pltPreview, &OGLViewWidget::initError, this, &CifCreatorFrame::processOpenGLError);
    } catch (const std::exception& e) {
        CLOG(WARNING, "gui") << "Failed to make OpenGL view: " << e.what();
        QMessageBox msgBox(this);
        msgBox.setText("Error:");
        msgBox.setInformativeText(e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setMinimumSize(160, 125);
        msgBox.exec();
    }
    
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

    connect(ui->btnPreview, &QPushButton::clicked, this, &CifCreatorFrame::previewStructure);
    connect(ui->cmbViewDirection, qOverload<const QString &>(&QComboBox::activated), this, &CifCreatorFrame::viewDirectionChanged);
}

void CifCreatorFrame::directionValuesChanged(QString dud) {
    double u = ui->edtZoneU->text().toDouble();
    double v = ui->edtZoneV->text().toDouble();
    double w = ui->edtZoneW->text().toDouble();
//    Eigen::Vector3d uvw;
//    uvw << u, v, w;

//    double a = ui->edtZoneA->text().toDouble();
//    double b = ui->edtZoneB->text().toDouble();
//    double c = ui->edtZoneC->text().toDouble();
//    Eigen::Vector3d abc;
//    abc << a, b, c;
//
//    uvw.normalize();
//    abc.normalize();

//    if(uvw != abc) { // I think this check is fine?
////        ui->edtZoneU->setStyleSheet("");
////        ui->edtZoneV->setStyleSheet("");
////        ui->edtZoneW->setStyleSheet("");
//        ui->edtZoneA->setStyleSheet("");
//        ui->edtZoneB->setStyleSheet("");
//        ui->edtZoneC->setStyleSheet("");
//    } else {
////        ui->edtZoneU->setStyleSheet("color: #FF8C00");
////        ui->edtZoneV->setStyleSheet("color: #FF8C00");
////        ui->edtZoneW->setStyleSheet("color: #FF8C00");
//        ui->edtZoneA->setStyleSheet("color: #FF8C00");
//        ui->edtZoneB->setStyleSheet("color: #FF8C00");
//        ui->edtZoneC->setStyleSheet("color: #FF8C00");
//        return;
//    }

    if (u != 0 || v != 0 || w != 0) {
        ui->edtZoneU->setStyleSheet("");
        ui->edtZoneV->setStyleSheet("");
        ui->edtZoneW->setStyleSheet("");
    } else {
        ui->edtZoneU->setStyleSheet("color: #FF8C00");
        ui->edtZoneV->setStyleSheet("color: #FF8C00");
        ui->edtZoneW->setStyleSheet("color: #FF8C00");
    }

    previewStructure();
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
    bool valid = dlgApply_clicked();
    if (valid) {
        auto * dlg = dynamic_cast<CifCreatorDialog *>(parentWidget());
        dlg->accept();
    }
}

bool CifCreatorFrame::dlgApply_clicked()
{
    bool valid = true;
    std::string errors;

    double xr = ui->edtRangeX->text().toDouble();
    double yr = ui->edtRangeY->text().toDouble();
    double zr = ui->edtRangeZ->text().toDouble();
    if (xr == 0 || yr == 0 || zr == 0) {
        valid = false;
        errors += "Structures ranges cannot be 0\n";
    }

    double u = ui->edtZoneU->text().toDouble();
    double v = ui->edtZoneV->text().toDouble();
    double w = ui->edtZoneW->text().toDouble();
    if (u != 0 && v != 0 && w != 0) {
        valid = false;
        errors += "Zone axis cannot be 000\n";
    }



    double a = ui->edtZoneA->text().toDouble();
    double b = ui->edtZoneB->text().toDouble();
    double c = ui->edtZoneC->text().toDouble();

    double ta = ui->edtTiltA->text().toDouble();
    double tb = ui->edtTiltB->text().toDouble();
    double tc = ui->edtTiltC->text().toDouble();

    if (!valid) {
        CLOG(WARNING, "gui") << "Failed to make cif supercell: " << errors;
        QMessageBox msgBox(this);
        msgBox.setText("Error:");
        msgBox.setInformativeText(QString::fromStdString(errors));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setMinimumSize(160, 125);
        msgBox.exec();
        return false;
    }

    CellInfo->setWidths(xr, yr, zr);
    CellInfo->setUVW(u, v, w);
    CellInfo->setABC(a, b, c);
    CellInfo->setABC(ta, tb, tc);



    return true;
}

void CifCreatorFrame::previewStructure(bool dummy) {

    if (!pltPreview)
        return;

    CIF::SuperCellInfo preview_info;

    // get all the values we need
    double u = ui->edtZoneU->text().toDouble();
    double v = ui->edtZoneV->text().toDouble();
    double w = ui->edtZoneW->text().toDouble();
    preview_info.setUVW(u, v, w);

    if (u == 0 && v == 0 && w == 0)
        return;

    double a = ui->edtZoneA->text().toDouble();
    double b = ui->edtZoneB->text().toDouble();
    double c = ui->edtZoneC->text().toDouble();
    preview_info.setABC(a, b, c);

    double ta = ui->edtTiltA->text().toDouble();
    double tb = ui->edtTiltB->text().toDouble();
    double tc = ui->edtTiltC->text().toDouble();
    preview_info.setTilts(ta, tb, tc);

    // TODO: maye just draw one unit cell?
    preview_info.setWidths(20, 20, 20);

    CrystalStructure temp(cif, preview_info);


    // get ranges (needed to define out 'cube'
    auto xr = temp.getLimitsX();
    auto yr = temp.getLimitsY();
    auto zr = temp.getLimitsZ();

    auto atms = temp.getAtoms();

    std::vector<Vector3f> pos(atms.size());
    std::vector<Vector3f> col(atms.size());

    for (int i = 0; i < atms.size(); ++i) {
        pos[i] = Vector3f(atms[i].x, atms[i].y, atms[i].z);

        auto qc = GuiUtils::ElementNumberToQColour(atms[i].A);
        col[i] = Vector3f(qc.red(), qc.green(), qc.blue()) / 255.0f;
    }

    pltPreview->PlotAtoms(pos, col, getViewDirection(), xr[0]+1, xr[1]-1, yr[0]+1, yr[1]-1, zr[0]+1, zr[1]-1);
}

View::Direction CifCreatorFrame::getViewDirection(){
    QString view_text = ui->cmbViewDirection->currentText();

    // set the view direcion of the plot
    if (view_text == "Top")
        return View::Direction::Top;
    else if (view_text == "Front")
        return View::Direction::Front;
    else if (view_text == "Right")
        return View::Direction::Right;
    else if (view_text == "Bottom")
        return View::Direction::Bottom;
    else if (view_text == "Back")
        return View::Direction::Back;
    else if (view_text == "Left")
        return View::Direction::Left;
}

void CifCreatorFrame::viewDirectionChanged() {
    if (!pltPreview)
        return;

    pltPreview->SetViewDirection(getViewDirection());

    pltPreview->repaint();
}

void CifCreatorFrame::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    previewStructure();
}

void CifCreatorFrame::processOpenGLError(std::string message) {
    CLOG(WARNING, "gui") << "OpenGL initialisation: " << message;
    QMessageBox msgBox(this);
    msgBox.setText("Error:");
    msgBox.setInformativeText(QString::fromStdString(message));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setMinimumSize(160, 125);
    msgBox.exec();
}
