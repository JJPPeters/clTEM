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

public slots:
    void updateTextBoxes();

private slots:
    void on_edtPosY_textChanged(const QString &arg1);

    void on_edtPosX_textChanged(const QString &arg1);

private:
    Ui::CbedFrame *ui;

    MainWindow* Main;
};

#endif // CBEDFRAME_H
