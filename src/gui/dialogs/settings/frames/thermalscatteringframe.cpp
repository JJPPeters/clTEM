#include <utils/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidgetItem>
#include "thermalscatteringframe.h"
#include "ui_thermalscatteringframe.h"

ThermalScatteringFrame::ThermalScatteringFrame(QWidget *parent) :
    QWidget(parent), ui(new Ui::ThermalScatteringFrame)
{
    ui->setupUi(this);

//    ui->tblDetectors->setColumnWidth(0, 120);
//    ui->tblDetectors->setColumnWidth(1, 70);
//    ui->tblDetectors->setColumnWidth(2, 70);
//    ui->tblDetectors->setColumnWidth(3, 70);
//    ui->tblDetectors->setColumnWidth(4, 70);
//
    ui->tblDisplacements->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//
//    ui->edtInner->setUnits("mrad");
//    ui->edtOuter->setUnits("mrad");
//    ui->edtCentreX->setUnits("mrad");
//    ui->edtCentreY->setUnits("mrad");
//
//    ui->edtInner->setText("0");
//    ui->edtOuter->setText("10");
//    ui->edtCentreX->setText("0");
//    ui->edtCentreY->setText("0");
//
//    connect(ui->edtInner, SIGNAL(textChanged(QString)), this, SLOT(doRadiiValid(QString)));
//    connect(ui->edtOuter, SIGNAL(textChanged(QString)), this, SLOT(doRadiiValid(QString)));
//
//    connect(parent, SIGNAL(okSignal()), this, SLOT(dlgOk_clicked()));
//    connect(parent, SIGNAL(cancelSignal()), this, SLOT(dlgCancel_clicked()));
//    connect(parent, SIGNAL(applySignal()), this, SLOT(dlgApply_clicked()));
//
//    for (const auto &chosenDetector : chosenDetectors)
//        addItemToList(chosenDetector);
//
//    setNewName();
}

ThermalScatteringFrame::~ThermalScatteringFrame()
{
    delete ui;
}

void ThermalScatteringFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    auto * dlg = dynamic_cast<ThermalScatteringDialog*>(parentWidget());
    dlg->reject();
}

void ThermalScatteringFrame::dlgOk_clicked()
{
//    // same as clicking apply then closing the dialog
//    if(dlgApply_clicked())
//    {
//        auto * dlg = dynamic_cast<StemDetectorDialog*>(parentWidget());
//        dlg->accept();
//    }
}

bool ThermalScatteringFrame::dlgApply_clicked()
{
//    chosenDetectors.clear();
//    int nRows = ui->tblDetectors->rowCount();
//    chosenDetectors.resize(static_cast<unsigned long>(nRows));
//
//    for (int i = 0; i < nRows; ++i)
//    {
//        std::string name = ui->tblDetectors->item(i, 0)->text().toStdString();
//        float inner = ui->tblDetectors->item(i, 1)->text().toFloat();
//        float outer = ui->tblDetectors->item(i, 2)->text().toFloat();
//        float xc = ui->tblDetectors->item(i, 3)->text().toFloat();
//        float yc = ui->tblDetectors->item(i, 4)->text().toFloat();
//
//        chosenDetectors[i] = StemDetector(name, inner, outer, xc, yc);
//    }
//
//    emit detectorsChanged();
//
//    return true;
}