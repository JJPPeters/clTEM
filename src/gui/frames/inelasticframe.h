#ifndef INELASTICFRAME_H
#define INELASTICFRAME_H

#include <QWidget>
#include <mainwindow.h>

namespace Ui {
class InelasticFrame;
}

class InelasticFrame : public QWidget
{
    Q_OBJECT

public:
    explicit InelasticFrame(QWidget *parent = 0);

    ~InelasticFrame();

    void assignMainWindow(MainWindow* m) {Main = m; updateGui();}

private slots:
    void checkEditZero(QString dud);

    void on_btnPhononMore_clicked();

    void on_btnPlasmonMore_clicked();

public slots:
    void updatePhononsGui();

    void updatePlasmonsGui();

    void updateIterationsGui();

    void updateGui() {updatePhononsGui(); updatePlasmonsGui(); updateIterationsGui();}

    void updatePhononsManager();

    void updatePlasmonsManager();

    void updateIterationsManager();

    void updateManager() {updatePhononsManager(); updatePlasmonsManager(); updateIterationsManager();}

private:
    Ui::InelasticFrame *ui;

    MainWindow* Main;
};

#endif // INELASTICFRAME_H