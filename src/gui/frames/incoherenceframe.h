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

    void assignMainWindow(MainWindow* m) {Main = m; updateTextBoxes();}


//private slots:
//    void checkEditZero(QString dud);

public slots:
    void updateTemTextBoxes();
    void updateTextBoxes() {updateTemTextBoxes();}

    void updateTemManager();
    void updateManager() {updateTemManager();}



private:
    Ui::IncoherenceFrame *ui;

    MainWindow* Main;
};

#endif // INCOHERENCEFRAME_H
