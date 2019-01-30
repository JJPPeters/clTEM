//
// Created by Jon on 30/01/2019.
//

#ifndef CLTEM_CIFCREATORFRAME_H
#define CLTEM_CIFCREATORFRAME_H

#include <QWidget>
#include <QtWidgets/QLineEdit>

#include <cif/supercell.h>

namespace Ui {
    class CifCreatorFrame;
}

class CifCreatorFrame : public QWidget
{
Q_OBJECT

public:
    explicit CifCreatorFrame(QWidget *, CIF::CIFReader _cif, std::shared_ptr<CIF::SuperCellInfo> _info);

private slots:
    void rangeValuesChanged(QString dud);

    void directionValuesChanged(QString dud);

private:
    Ui::CifCreatorFrame *ui;

    std::shared_ptr<CIF::SuperCellInfo> CellInfo;

    CIF::CIFReader cif;

    void dlgCancel_clicked();

    void dlgOk_clicked();

    void dlgApply_clicked();
};


#endif //CLTEM_CIFCREATORFRAME_H
