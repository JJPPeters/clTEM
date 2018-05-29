#include <utils/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidgetItem>
#include <structure/thermalvibrations.h>
#include "thermalscatteringframe.h"
#include "ui_thermalscatteringframe.h"

ThermalScatteringFrame::ThermalScatteringFrame(QWidget *parent) :
    QWidget(parent), ui(new Ui::ThermalScatteringFrame)
{
    ui->setupUi(this);

    // fiddle with the table
    ui->tblDisplacements->setColumnWidth(0, 80);
    ui->tblDisplacements->setColumnWidth(1, 200);
    ui->tblDisplacements->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    // sort out text boxes units and so on
    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtDefault->setValidator(pValidator);
    ui->edtDisplacement->setValidator(pValidator);

    ui->edtDefault->setUnits("Å²");
    ui->edtDisplacement->setUnits("Å²");

    // set the text box values
    ui->edtDefault->setText( QString::number( ThermalVibrations::getDefault() ) );
    ui->edtDisplacement->setText("0.0");

    // fill the combo box from our map of atomic numbers to symbols
    // http://www.cplusplus.com/forum/beginner/123379/

    for (const auto &it : Utils::VectorSymbolToNumber) {
        ui->cmbElement->addItem( QString::fromStdString(it.first));
    }


//    connect(ui->edtInner, SIGNAL(textChanged(QString)), this, SLOT(doRadiiValid(QString)));
//    connect(ui->edtOuter, SIGNAL(textChanged(QString)), this, SLOT(doRadiiValid(QString)));

    // connect up our OK, etc... buttons
    connect(parent, SIGNAL(okSignal()), this, SLOT(dlgOk_clicked()));
    connect(parent, SIGNAL(cancelSignal()), this, SLOT(dlgCancel_clicked()));
    connect(parent, SIGNAL(applySignal()), this, SLOT(dlgApply_clicked()));

//    for (const auto &chosenDetector : chosenDetectors)
//        addItemToList(chosenDetector);
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
    // same as clicking apply then closing the dialog
    if(dlgApply_clicked())
    {
        auto * dlg = dynamic_cast<ThermalScatteringDialog*>(parentWidget());
        dlg->accept();
    }
}

bool ThermalScatteringFrame::dlgApply_clicked()
{
    // get our default, easy...
    float def = ui->edtDefault->text().toFloat();

    // create vectors of our explicitly defined elements
    int nRows = ui->tblDisplacements->rowCount();

    std::vector<int> elements(nRows);
    std::vector<float> displacements(nRows);

    for (int i = 0; i < nRows; ++i) {
        std::string temp_el = ui->tblDisplacements->item(i, 0)->text().toStdString();
        elements[i] = Utils::ElementSymbolToNumber(temp_el);
        displacements[i] = ui->tblDisplacements->item(i, 1)->text().toFloat();
    }

    ThermalVibrations::setVibrations(def, elements, displacements);

    return true;
}