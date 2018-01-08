#ifndef ABERRATIONFORM_H
#define ABERRATIONFORM_H

#include <QWidget>
#include <mainwindow.h>

namespace Ui {
class AberrationForm;
}

class AberrationFrame : public QWidget
{
    Q_OBJECT

public:
    explicit AberrationFrame(QWidget *parent = 0);

    ~AberrationFrame();

    void assignMainWindow(MainWindow* m) {Main = m;}

    void updateAberrations();

private slots:
    void checkEditZero(QString dud);

    void on_btnMore_clicked();

public slots:
    void updateTextBoxes();

private:
    Ui::AberrationForm *ui;

    MainWindow* Main;
};

#endif // ABERRATIONFORM_H
