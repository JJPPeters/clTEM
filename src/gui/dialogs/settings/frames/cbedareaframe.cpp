#include <utility>

#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include "cbedareaframe.h"
#include "ui_cbedareaframe.h"

CbedAreaFrame::CbedAreaFrame(QWidget *parent, CbedPosition pos, std::shared_ptr<CrystalStructure> struc) :
    QWidget(parent), ui(new Ui::CbedAreaFrame), Position(pos), Structure(std::move(struc))
{
    ui->setupUi(this);

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));
    QRegExpValidator* pValidator = new QRegExpValidator(QRegExp(R"([+]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtPosX->setValidator(pmValidator);
    ui->edtPosY->setValidator(pmValidator);

    ui->edtPadding->setValidator(pValidator);

    ui->edtPosX->setUnits("Å");
    ui->edtPosY->setUnits("Å");
    ui->edtPadding->setUnits("Å");

    connect(ui->edtPadding, &QLineEdit::textChanged, this, &CbedAreaFrame::valuesChanged);

    connect(ui->edtPosX, &QLineEdit::editingFinished, this, &CbedAreaFrame::editing_finished);
    connect(ui->edtPosY, &QLineEdit::editingFinished, this, &CbedAreaFrame::editing_finished);
    connect(ui->edtPadding, &QLineEdit::editingFinished, this, &CbedAreaFrame::editing_finished);

    // this just resets the values to the currently stored ones
    on_btnReset_clicked();
}

CbedAreaFrame::~CbedAreaFrame()
{
    delete ui;
}

void CbedAreaFrame::valuesChanged(QString dud) {
    emit areaChanged();
}

CbedPosition CbedAreaFrame::getCbedPos() {
    double xp = ui->edtPosX->text().toDouble();
    double yp = ui->edtPosY->text().toDouble();
    double padding = ui->edtPadding->text().toDouble();
    return CbedPosition(xp, yp, padding);
}

void CbedAreaFrame::on_btnReset_clicked()
{
    ui->edtPosX->setText(Utils_Qt::numToQString( Position.getXPos()));
    ui->edtPosY->setText(Utils_Qt::numToQString( Position.getYPos()));
    ui->edtPadding->setText(Utils_Qt::numToQString( Position.getPadding()));
    emit areaChanged();
}

void CbedAreaFrame::on_btnDefault_clicked() {
    //TODO: this default is set in the json file -> get it here somehow?
    if (Structure) {
        auto xLims = Structure->limitsX();
        auto yLims = Structure->limitsY();
        double padding = 0.f;

        ui->edtPosX->setText(Utils_Qt::numToQString((xLims[0] + xLims[1]) / 2));
        ui->edtPosY->setText(Utils_Qt::numToQString((yLims[0] + yLims[1]) / 2));

        ui->edtPadding->setText(Utils_Qt::numToQString(padding));
    } else {
        on_btnReset_clicked();
    }
    emit areaChanged();
}

void CbedAreaFrame::editing_finished() {
    auto* sndr = (QLineEdit*) sender();

    auto val = sndr->text().toDouble();
    sndr->setText(Utils_Qt::numToQString( val ));
}
