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

    void assignMainWindow(MainWindow* m) {Main = m;}

    bool isTdsEnabled();

    unsigned int getTdsRuns();

    void setActive(bool active);

public slots:
    void updateScaleLabels();

private slots:
    void on_btnDetectors_clicked();

    void on_btnArea_clicked();

    void on_btnSim_clicked();

    void on_btnCancel_clicked();

    void updateDetectors();

    void on_edtTds_textChanged(const QString &arg1);

private:
    Ui::StemFrame *ui;

    MainWindow* Main;
};

#endif // STEMFRAME_H
