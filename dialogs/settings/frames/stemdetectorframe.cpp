#include <utilities/stringutils.h>
#include <dialogs/settings/settingsdialog.h>
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

    connect(ui->edtInner, SIGNAL(textChanged(QString)), this, SLOT(doRadiiValid(QString)));
    connect(ui->edtOuter, SIGNAL(textChanged(QString)), this, SLOT(doRadiiValid(QString)));

    connect(parent, SIGNAL(okSignal()), this, SLOT(dlgOk_clicked()));
    connect(parent, SIGNAL(cancelSignal()), this, SLOT(dlgCancel_clicked()));
    connect(parent, SIGNAL(applySignal()), this, SLOT(dlgApply_clicked()));

    for(int i = 0; i < chosenDetectors.size(); ++i)
        addItemToList(chosenDetectors[i]);

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
    for(int i = 0; i < selection.size(); ++i)
        if(selection.at(i)->column() == 0)
            toRemove.push_back(selection.at(i)->row());

    std::sort(toRemove.begin(), toRemove.end());

    int n = 0;
    for(int i = 0; i < toRemove.size(); ++i)
    {
        ui->tblDetectors->removeRow(toRemove[i] - n);
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
    if(name == "" || name == "Img" || name == "EW A" || name == "EW θ")
        return false;

    int n = ui->tblDetectors->rowCount();

    for (int i = 0; i < n; ++i)
        if(name == ui->tblDetectors->item(i, 0)->text().toStdString())
            return false;

    return true;
}

bool StemDetectorFrame::checkRadiiValid()
{
    if(ui->edtInner->text().toFloat() >= ui->edtOuter->text().toFloat())
        return false;
    return true;
}

void StemDetectorFrame::addItemToList(StemDetector det)
{
    int n = ui->tblDetectors->rowCount();
    ui->tblDetectors->insertRow(n);

    QTableWidgetItem* cell_0 = new QTableWidgetItem();
    cell_0->setTextAlignment(Qt::AlignCenter);
    cell_0->setText(QString::fromStdString(det.name));

    auto cell_1 = cell_0->clone();
    cell_1->setText(QString::fromStdString(Utils::numToString(det.inner)));

    auto cell_2 = cell_0->clone();
    cell_2->setText(QString::fromStdString(Utils::numToString(det.outer)));

    auto cell_3 = cell_0->clone();
    cell_3->setText(QString::fromStdString(Utils::numToString(det.xcentre)));

    auto cell_4 = cell_0->clone();
    cell_4->setText(QString::fromStdString(Utils::numToString(det.ycentre)));

    ui->tblDetectors->setItem(n, 0, cell_0);
    ui->tblDetectors->setItem(n, 1, cell_1);
    ui->tblDetectors->setItem(n, 2, cell_2);
    ui->tblDetectors->setItem(n, 3, cell_3);
    ui->tblDetectors->setItem(n, 4, cell_4);
}

void StemDetectorFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    StemDetectorDialog* dlg = static_cast<StemDetectorDialog*>(parentWidget());
    dlg->reject();
}

void StemDetectorFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    if(dlgApply_clicked())
    {
        StemDetectorDialog* dlg = static_cast<StemDetectorDialog*>(parentWidget());
        dlg->accept();
    }
}

bool StemDetectorFrame::dlgApply_clicked()
{
    chosenDetectors.clear();
    int nRows = ui->tblDetectors->rowCount();
    chosenDetectors.resize(nRows);

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
