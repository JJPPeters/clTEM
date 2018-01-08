#ifndef STEMAREAFRAME_H
#define STEMAREAFRAME_H

#include <QWidget>
#include <utilities/commonstructs.h>
#include <QtWidgets/QLineEdit>

namespace Ui {
class StemAreaFrame;
}

class StemAreaFrame : public QWidget
{
    Q_OBJECT

signals:
    void areaChanged();

public:
    explicit StemAreaFrame(QWidget *parent, std::shared_ptr<StemArea> stem, std::shared_ptr<SimulationArea> sim);

    ~StemAreaFrame();

private slots:
    void on_btnReset_clicked();

    void checkEditZero(QString dud);

    void checkValidX(QString dud);
    void checkValidY(QString dud);

    void checkRangeValid(QLineEdit* start, QLineEdit* finish, float simStart, float simFinish);

    void dlgOk_clicked();

    bool dlgApply_clicked();

    void dlgCancel_clicked();

private:
    Ui::StemAreaFrame *ui;

    std::shared_ptr<StemArea> stemSimArea;

    std::shared_ptr<SimulationArea> simArea;
};

#endif // STEMAREAFRAME_H
