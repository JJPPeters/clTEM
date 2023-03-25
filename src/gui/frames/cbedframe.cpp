#include "cbedframe.h"
#include "ui_cbedframe.h"

#include <QtGui/QRegExpValidator>
#include <utils/stringutils.h>
#include <QScreen>

CbedFrame::CbedFrame(QWidget *parent) :
        QWidget(parent), ui(new Ui::CbedFrame), Main(0)
{
    ui->setupUi(this);

    QRegExpValidator* pmValidator = new QRegExpValidator(QRegExp(R"([+-]?(\d*(?:\.\d*)?(?:[eE]([+\-]?\d+)?)>)*)"));

    ui->edtPosX->setValidator(pmValidator);
    ui->edtPosY->setValidator(pmValidator);

    ui->edtPosX->setUnits("Å");
    ui->edtPosY->setUnits("Å");
}

CbedFrame::~CbedFrame()
{
    delete ui;
}

void CbedFrame::on_edtPosY_textChanged(const QString &arg1)
{
    (void)arg1; // don't want this

    if (Main == nullptr)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    double v = ui->edtPosY->text().toDouble();

    Main->Manager->cbedPosition()->setYPos(v);
}

void CbedFrame::on_edtPosX_textChanged(const QString &arg1)
{
    (void)arg1; // don't want this

    if (Main == nullptr)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    double v = ui->edtPosX->text().toDouble();

    Main->Manager->cbedPosition()->setXPos(v);
}

void CbedFrame::updateTextBoxes()
{
    if (Main == 0)
        throw std::runtime_error("Error connecting CBED frame to main window.");

    ui->edtPosX->setText( Utils_Qt::numToQString(Main->Manager->cbedPosition()->getXPos()) );
    ui->edtPosY->setText( Utils_Qt::numToQString(Main->Manager->cbedPosition()->getYPos()) );
}