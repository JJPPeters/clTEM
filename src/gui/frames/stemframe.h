#ifndef STEMFRAME_H
#define STEMFRAME_H

#include <QWidget>
#include <mainwindow.h>

class MainWindow;

namespace Ui {
class StemFrame;
}

class StemFrame : public QWidget
{
    Q_OBJECT

signals:
    void startSim();
    void stopSim();

public:
    explicit StemFrame(QWidget *parent = 0);

    ~StemFrame();

    void assignMainWindow(MainWindow* m) {Main = m; updateScaleLabels(); updateTextBoxes();}

    void setTdsEnabled(bool enabled);

    void setTdsRuns(unsigned int runs);

    bool isTdsEnabled();

    unsigned int getTdsRuns();

    void setActive(bool active);

public slots:
    void updateScaleLabels();

    void updateTextBoxes();

    void updateTds();

private slots:
    void on_btnDetectors_clicked();

    void on_btnArea_clicked();

    void on_btnSim_clicked();

    void on_btnCancel_clicked();

    void updateDetectors();

    void edtTds_changed(const QString &arg1);

    void on_chkTds_stateChanged(int state);

private:
    Ui::StemFrame *ui;

    MainWindow* Main;

    void edtTds_changed_proxy(const QString &arg1, bool update_partner);
};

#endif // STEMFRAME_H
