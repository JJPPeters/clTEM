#ifndef INCOHERENCEFRAME_H
#define INCOHERENCEFRAME_H

#include <QWidget>
#include <mainwindow.h>

namespace Ui {
class IncoherenceFrame;
}

class IncoherenceFrame : public QWidget
{
    Q_OBJECT

public:
    explicit IncoherenceFrame(QWidget *parent = 0);

    ~IncoherenceFrame();

//    void assignMainWindow(MainWindow* m) {Main = m; updateTextBoxes();}

//    void updateAberrations();

//private slots:
//    void checkEditZero(QString dud);

//    void on_btnMore_clicked();

//    void on_edtVoltage_textChanged(const QString &arg1);

//public slots:
//    void updateTextBoxes();

private:
    Ui::IncoherenceFrame *ui;

    MainWindow* Main;
};

#endif // INCOHERENCEFRAME_H
