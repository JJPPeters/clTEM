#ifndef FULLABERRATIONFRAME_H
#define FULLABERRATIONFRAME_H

#include <QWidget>
#include <utilities/commonstructs.h>
#include <simulationmanager.h>

namespace Ui {
class FullAberrationFrame;
}

class FullAberrationFrame : public QWidget
{
    Q_OBJECT

public:
    explicit FullAberrationFrame(QWidget *parent, std::shared_ptr<SimulationManager> params);
    ~FullAberrationFrame();

private:
    Ui::FullAberrationFrame *ui;

    void setValidators();

    void setValues();

    void setUnits();

    std::shared_ptr<SimulationManager> Manager;

    void setBackgroundStyles();

private slots:
    void checkEditZero(QString dud);

    void dlgOk_clicked();

    bool dlgApply_clicked();

    void dlgCancel_clicked();

};

#endif // FULLABERRATIONFRAME_H
