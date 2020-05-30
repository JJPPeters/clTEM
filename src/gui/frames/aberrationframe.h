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

    void assignMainWindow(MainWindow* m) {
        Main = m;
        setModeStyles(m->Manager->mode(), m->Manager->ctemImageEnabled());
        updateTextBoxes();
    }

    void updateAberrations();

private slots:
    void on_btnMore_clicked();

public slots:
    void updateTextBoxes();

    void setModeStyles(SimulationMode md, bool tem_image);

private:
    Ui::AberrationFrame *ui;

    MainWindow* Main;
};

#endif // ABERRATIONFRAME_H
