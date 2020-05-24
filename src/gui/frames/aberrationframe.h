#ifndef ABERRATIONFRAME_H
#define ABERRATIONFRAME_H

#include <QWidget>
#include <mainwindow.h>

namespace Ui {
class AberrationFrame;
}

class AberrationFrame : public QWidget
{
    Q_OBJECT

public:
    explicit AberrationFrame(QWidget *parent = 0);

    ~AberrationFrame();

    void assignMainWindow(MainWindow* m) {Main = m; updateTextBoxes();}

    void updateAberrations();

private slots:
    void checkEditZero(QString dud);

    void on_btnMore_clicked();

public slots:
    void updateTextBoxes();

private:
    Ui::AberrationFrame *ui;

    MainWindow* Main;
};

#endif // ABERRATIONFRAME_H
