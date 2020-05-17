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

    void assignMainWindow(MainWindow* m) {Main = m; updateTextBoxes();}

    void setTdsEnabled(bool enabled);

    void setTdsRuns(unsigned int runs);

    bool isTdsEnabled();

    unsigned int getTdsRuns();

    void setActive(bool active);

public slots:
    void updateTextBoxes();

    void updateTds();

private slots:
    void edtTds_changed(const QString &arg1);

    void on_edtPosY_textChanged(const QString &arg1);

    void on_edtPosX_textChanged(const QString &arg1);

    void on_btnSim_clicked();

    void on_btnCancel_clicked();

    void on_chkTds_stateChanged(int state);

private:
    Ui::CbedFrame *ui;

    MainWindow* Main;

    void edtTds_changed_proxy(const QString &arg1, bool update_partner);
};

#endif // CBEDFRAME_H
