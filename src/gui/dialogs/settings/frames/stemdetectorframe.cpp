#include <utils/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidgetItem>
#include "stemdetectorframe.h"
#include "ui_stemdetectorframe.h"

StemDetectorFrame::StemDetectorFrame(QWidget *parent, std::vector<StemDetector>& dets) :
    QWidget(parent), chosenDetectors(dets),
    ui(new Ui::StemDetectorFrame)
{
    ui->setupUi(this);

    ui->tblDetectors->setColumnWidth(0, 120);
    ui->tblDetectors->setColumnWidth(1, 70);
    ui->tblDetectors->setColumnWidth(2, 70);
    ui->tblDetectors->setColumnWidth(3, 70);
    ui->tblDetectors->setColumnWidth(4, 70);

    ui->tblDetectors->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->edtInner->setUnits("mrad");
    ui->edtOuter->setUnits("mrad");
    ui->edtCentreX->setUnits("mrad");
    ui->edtCentreY->setUnits("mrad");

    ui->edtInner->setText("0");
    ui->edtOuter->setText("10");
    ui->edtCentreX->setText("0");
    ui->edtCentreY->setText("0");

    connect(ui->edtInner, SIGNAL(textChanged(QString)), this, SLOT(doRadiiValid(QString)));
    connect(ui->edtOuter, SIGNAL(textChanged(QString)), this, SLOT(doRadiiValid(QString)));

    connect(parent, SIGNAL(okSignal()), this, SLOT(dlgOk_clicked()));
    connect(parent, SIGNAL(cancelSignal()), this, SLOT(dlgCancel_clicked()));
    connect(parent, SIGNAL(applySignal()), this, SLOT(dlgApply_clicked()));

    for (const auto &chosenDetector : chosenDetectors)
        addItemToList(chosenDetector);

    setNewName();
}

StemDetectorFrame::~StemDetectorFrame()
{
    delete ui;
}

void StemDetectorFrame::on_btnAdd_clicked()
{
    // test that the outer radius is greater then the inner
    if(!checkRadiiValid())
        return; //TODO: maybe a dialog here?

    std::string name = ui->edtName->text().toStdString();

    if(!checkNameValid(name))
        return;

    float inner = ui->edtInner->text().toFloat();
    float outer = ui->edtOuter->text().toFloat();
    float xc = ui->edtCentreX->text().toFloat();
    float yc = ui->edtCentreY->text().toFloat();

    StemDetector d(name, inner, outer, xc, yc);

    addItemToList(d);

    setNewName();
}

void StemDetectorFrame::on_btnDelete_clicked()
{
    QList<QTableWidgetItem *> selection = ui->tblDetectors->selectedItems();

    std::vector<int> toRemove;
    for (auto i : selection)
        if(i->column() == 0)
            toRemove.push_back(i->row());

    std::sort(toRemove.begin(), toRemove.end());

    int n = 0;
    for (int i : toRemove) {
        ui->tblDetectors->removeRow(i - n);
        ++n;
    }

    setNewName();
}

void StemDetectorFrame::on_edtName_textChanged(const QString &arg1)
{
    if(!checkNameValid(arg1.toStdString()))
        ui->edtName->setStyleSheet("color: #FF8C00");
    else
        ui->edtName->setStyleSheet("");
}

bool StemDetectorFrame::checkNameValid(std::string name)
{
    if(name.empty() || name == "Img" || name == "EW A" || name == "EW θ")
        return false;

    int n = ui->tblDetectors->rowCount();

    for (int i = 0; i < n; ++i)
        if(name == ui->tblDetectors->item(i, 0)->text().toStdString())
            return false;

    return true;
}

bool StemDetectorFrame::checkRadiiValid()
{
    return ui->edtInner->text().toFloat() < ui->edtOuter->text().toFloat();
}

void StemDetectorFrame::addItemToList(StemDetector det)
{
    int n = ui->tblDetectors->rowCount();
    ui->tblDetectors->insertRow(n);

    auto * cell_0 = new QTableWidgetItem();
    cell_0->setTextAlignment(Qt::AlignCenter);
    cell_0->setText(QString::fromStdString(det.name));

    auto cell_1 = cell_0->clone();
    cell_1->setText(Utils_Qt::numToQString(det.inner));

    auto cell_2 = cell_0->clone();
    cell_2->setText(Utils_Qt::numToQString(det.outer));

    auto cell_3 = cell_0->clone();
    cell_3->setText(Utils_Qt::numToQString(det.xcentre));

    auto cell_4 = cell_0->clone();
    cell_4->setText(Utils_Qt::numToQString(det.ycentre));

    ui->tblDetectors->setItem(n, 0, cell_0);
    ui->tblDetectors->setItem(n, 1, cell_1);
    ui->tblDetectors->setItem(n, 2, cell_2);
    ui->tblDetectors->setItem(n, 3, cell_3);
    ui->tblDetectors->setItem(n, 4, cell_4);
}

void StemDetectorFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    auto * dlg = dynamic_cast<StemDetectorDialog*>(parentWidget());
    dlg->reject();
}

void StemDetectorFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    if(dlgApply_clicked())
    {
        auto * dlg = dynamic_cast<StemDetectorDialog*>(parentWidget());
        dlg->accept();
    }
}

bool StemDetectorFrame::dlgApply_clicked()
{
    chosenDetectors.clear();
    int nRows = ui->tblDetectors->rowCount();
    chosenDetectors.resize(static_cast<unsigned long>(nRows));

    for (int i = 0; i < nRows; ++i)
    {
        std::string name = ui->tblDetectors->item(i, 0)->text().toStdString();
        float inner = ui->tblDetectors->item(i, 1)->text().toFloat();
        float outer = ui->tblDetectors->item(i, 2)->text().toFloat();
        float xc = ui->tblDetectors->item(i, 3)->text().toFloat();
        float yc = ui->tblDetectors->item(i, 4)->text().toFloat();

        chosenDetectors[i] = StemDetector(name, inner, outer, xc, yc);
    }

    emit detectorsChanged();

    return true;
}

void StemDetectorFrame::doRadiiValid(QString dud)
{
    (void)dud; // don't need this

    if (!checkRadiiValid())
    {
        ui->edtInner->setStyleSheet("color: #FF8C00");
        ui->edtOuter->setStyleSheet("color: #FF8C00");
    }
    else
    {
        ui->edtInner->setStyleSheet("");
        ui->edtOuter->setStyleSheet("");
    }
}

void StemDetectorFrame::setNewName()
{
    bool valid = false;
    int i = 0;
    std::string newDet;
    while(!valid)
    {
        newDet = "Detector " + Utils::numToString(i);
        valid = checkNameValid(newDet);
        ++i;
    }
    ui->edtName->setText(QString::fromStdString(newDet));
}
