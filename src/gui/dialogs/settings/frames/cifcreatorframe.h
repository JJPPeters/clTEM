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
    explicit CifCreatorFrame(QWidget *parent, CIF::CIFReader _cif, std::shared_ptr<CIF::SuperCellInfo> _info);

private slots:
    void rangeValuesChanged(QString dud);

    void directionValuesChanged(QString dud);

    void showEvent(QShowEvent* event) override;

    void previewStructure(bool dummy = false);

    void viewDirectionChanged();

    void processOpenGLError(std::string message);

private:
    Ui::CifCreatorFrame *ui;

    std::shared_ptr<PGL::PlotWidget> pltPreview;

    std::shared_ptr<CIF::SuperCellInfo> CellInfo;

    CIF::CIFReader cif;

    View::Direction getViewDirection();

    void dlgCancel_clicked();

    void dlgOk_clicked();

    bool dlgApply_clicked();
};


#endif //CLTEM_CIFCREATORFRAME_H
