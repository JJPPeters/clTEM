//
// Created by Jon on 30/01/2019.
//

#include <utils/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include "cifcreatorframe.h"
#include "ui_cifcreatorframe.h"
#include <QScreen>

CifCreatorFrame::CifCreatorFrame(QWidget *parent, CIF::CIFReader _cif, std::shared_ptr<CIF::SuperCellInfo> _info) : QWidget(parent), ui(new Ui::CifCreatorFrame), CellInfo(_info), cif(_cif) {
    ui->setupUi(this);

    // This is not needed, but it might be needed if the computation turns out to be too taxing for some systems
    ui->btnPreview->setVisible(false);
    // Set up the OpenGL stuffs

    auto lb = new QLabel(this);
    lb->setText("Preview");
    lb->setAlignment(Qt::AlignCenter);
    ui->vPlotLayout->addWidget(lb, 0);

    try {
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setVersion(4, 0); // sets opengl version

        QSettings settings;
        int msaa = settings.value("MSAA", 1).toInt();

        pltPreview = std::make_shared<PGL::PlotWidget>(this, msaa);
        pltPreview->setFormat(format);

        ui->vPlotLayout->addWidget(pltPreview.get(), 1);

        QScreen* primary_screen = QGuiApplication::primaryScreen();
        double pixel_ratio = primary_screen->devicePixelRatio();
        pltPreview->setMinimumHeight(400 / pixel_ratio);
        pltPreview->setMinimumWidth(400 / pixel_ratio);

        pltPreview->setAttribute(Qt::WA_TransparentForMouseEvents);
        connect(pltPreview.get(), &PGL::PlotWidget::initError, this, &CifCreatorFrame::processOpenGLError);
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

    connect(ui->edtTiltA, &QLineEdit::textChanged, this, &CifCreatorFrame::angleValuesChanged);
    connect(ui->edtTiltB, &QLineEdit::textChanged, this, &CifCreatorFrame::angleValuesChanged);
    connect(ui->edtTiltC, &QLineEdit::textChanged, this, &CifCreatorFrame::angleValuesChanged);

    connect(ui->btnPreview, &QPushButton::clicked, this, &CifCreatorFrame::previewStructure);
    connect(ui->cmbViewDirection, qOverload<const QString &>(&QComboBox::activated), this, &CifCreatorFrame::viewDirectionChanged);
}

void CifCreatorFrame::angleValuesChanged(QString dud) {
    double ta = ui->edtTiltA->text().toDouble();
    double tb = ui->edtTiltB->text().toDouble();
    double tc = ui->edtTiltC->text().toDouble();

    if (pltCellInfo.equalTilts(ta, tb, tc))
        return;

    previewStructure();
}

void CifCreatorFrame::directionValuesChanged(QString dud) {
    double u = ui->edtZoneU->text().toDouble();
    double v = ui->edtZoneV->text().toDouble();
    double w = ui->edtZoneW->text().toDouble();

    auto parent_dlg = dynamic_cast<CifCreatorDialog*>(parentWidget());

    if (u != 0 || v != 0 || w != 0) {
        parent_dlg->setOkEnabled(true);
        ui->edtZoneU->setStyleSheet("");
        ui->edtZoneV->setStyleSheet("");
        ui->edtZoneW->setStyleSheet("");
    } else {
        parent_dlg->setOkEnabled(false);
        ui->edtZoneU->setStyleSheet("color: #FF8C00");
        ui->edtZoneV->setStyleSheet("color: #FF8C00");
        ui->edtZoneW->setStyleSheet("color: #FF8C00");
    }

    previewStructure();
}

void CifCreatorFrame::rangeValuesChanged(QString dud) {

    auto parent_dlg = dynamic_cast<CifCreatorDialog*>(parentWidget());

    bool valid = true;

    if (ui->edtRangeX->text().toDouble() != 0)
        ui->edtRangeX->setStyleSheet("");
    else {
        ui->edtRangeX->setStyleSheet("color: #FF8C00");
        valid = false;
    }

    if (ui->edtRangeY->text().toDouble() != 0)
        ui->edtRangeY->setStyleSheet("");
    else {
        ui->edtRangeY->setStyleSheet("color: #FF8C00");
        valid = false;
    }

    if (ui->edtRangeZ->text().toDouble() != 0)
        ui->edtRangeZ->setStyleSheet("");
    else {
        ui->edtRangeZ->setStyleSheet("color: #FF8C00");
        valid = false;
    }

    parent_dlg->setOkEnabled(valid);
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
    if (u == 0 && v == 0 && w == 0) {
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
    CellInfo->setTilts(ta, tb, tc);

    return true;
}

void CifCreatorFrame::previewStructure(bool dummy) {

    if (!pltPreview)
        return;

    pltPreview->clearItems();

//    bool changed = false;

    // get all the values we need
    double u = ui->edtZoneU->text().toDouble();
    double v = ui->edtZoneV->text().toDouble();
    double w = ui->edtZoneW->text().toDouble();
    pltCellInfo.setUVW(u, v, w);

    if (u == 0 && v == 0 && w == 0)
        return;

    double a = ui->edtZoneA->text().toDouble();
    double b = ui->edtZoneB->text().toDouble();
    double c = ui->edtZoneC->text().toDouble();
    pltCellInfo.setABC(a, b, c);

    double ta = ui->edtTiltA->text().toDouble();
    double tb = ui->edtTiltB->text().toDouble();
    double tc = ui->edtTiltC->text().toDouble();
    pltCellInfo.setTilts(ta, tb, tc);

    // TODO: maye just draw one unit cell?
    pltCellInfo.setWidths(22, 22, 22);

    auto parent_dlg = dynamic_cast<CifCreatorDialog*>(parentWidget());

    try {
        CrystalStructure temp(cif, pltCellInfo);

        // get ranges (needed to define out 'cube'
        auto xr = temp.limitsX();
        auto yr = temp.limitsY();
        auto zr = temp.limitsZ();

        auto atms = temp.atoms();

        std::vector<Eigen::Vector3f> pos(atms.size());
        std::vector<Eigen::Vector3f> col(atms.size());

        for (size_t i = 0; i < atms.size(); ++i) {
            pos[i] = Eigen::Vector3f(atms[i].x, atms[i].y, atms[i].z);

            auto qc = GuiUtils::ElementNumberToQColour(atms[i].A);
            col[i] = Eigen::Vector3f(qc.red(), qc.green(), qc.blue()) / 255.0;
        }

        pltPreview->scatter(pos, col);
//        pltPreview->SetViewDirection(View::Direction::Top);

        parent_dlg->setOkEnabled(true);
        ui->edtZoneU->setStyleSheet("");
        ui->edtZoneV->setStyleSheet("");
        ui->edtZoneW->setStyleSheet("");
        //
        ui->edtZoneA->setStyleSheet("");
        ui->edtZoneB->setStyleSheet("");
        ui->edtZoneC->setStyleSheet("");
    } catch (const std::exception& e) {
        parent_dlg->setOkEnabled(false);
        ui->edtZoneU->setStyleSheet("color: #FF8C00");
        ui->edtZoneV->setStyleSheet("color: #FF8C00");
        ui->edtZoneW->setStyleSheet("color: #FF8C00");
        //
        ui->edtZoneA->setStyleSheet("color: #FF8C00");
        ui->edtZoneB->setStyleSheet("color: #FF8C00");
        ui->edtZoneC->setStyleSheet("color: #FF8C00");
    }

    pltPreview->FitView(0.8);
    pltPreview->repaint();
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

    return View::Direction::Front; // jsut some default
}

void CifCreatorFrame::viewDirectionChanged() {
    if (!pltPreview)
        return;

    pltPreview->SetViewDirection(getViewDirection());
    pltPreview->FitView(0.8);

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
