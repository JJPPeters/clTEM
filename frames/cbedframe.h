#ifndef CBEDFRAME_H
#define CBEDFRAME_H

#include <QWidget>
#include <mainwindow.h>

namespace Ui {
class CbedFrame;
}

class CbedFrame : public QWidget
{
    Q_OBJECT

signals:
    void startSim();
    void stopSim();

public:
    explicit CbedFrame(QWidget *parent = 0);

    ~CbedFrame();

    void assignMainWindow(MainWindow* m) {Main = m; update_text_boxes();}

    bool isTdsEnabled();

    unsigned int getTdsRuns();

    void setActive(bool active);

private slots:
    void on_edtTds_textChanged(const QString &arg1);

    void on_edtPosY_textChanged(const QString &arg1);

    void on_edtPosX_textChanged(const QString &arg1);

    void on_btnSim_clicked();

    void on_btnCancel_clicked();

    void on_chkTds_stateChanged(int state);

    void update_text_boxes();

private:
    Ui::CbedFrame *ui;

    MainWindow* Main;
};

#endif // CBEDFRAME_H
