#include "statuslayout.h"
#include "ui_statuslayout.h"

StatusLayout::StatusLayout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusLayout)
{
    ui->setupUi(this);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar_2->setMinimum(0);
    ui->progressBar_2->setMaximum(100);
}

StatusLayout::~StatusLayout()
{
    delete ui;
}

void StatusLayout::setSliceProgress(double prog)
{
    ui->progressBar->setValue(static_cast<int>(prog*100));
}

void StatusLayout::setTotalProgress(double prog)
{
    ui->progressBar_2->setValue(static_cast<int>(prog*100));
}

void StatusLayout::setFileLabel(const QString &lbl) {
    ui->lblFile->setText(lbl);
}
