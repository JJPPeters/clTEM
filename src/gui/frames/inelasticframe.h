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
    void on_btnPhononMore_clicked();

    void on_btnPlasmonMore_clicked();

public slots:
    void updatePhononsGui();

    void updatePlasmonsGui();

    void updateGui() {updatePhononsGui(); updatePlasmonsGui();}

    void updatePhononsManager();

    void updatePlasmonsManager();

    void updateManager() {updatePhononsManager(); updatePlasmonsManager();}

private:
    Ui::InelasticFrame *ui;

    MainWindow* Main;
};

#endif // INELASTICFRAME_H
