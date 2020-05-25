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

    void assignMainWindow(MainWindow* m) {
        Main = m;
        setModeStyles(m->Manager->mode(), m->Manager->ctemImageEnabled());
        updateTextBoxes();
    }


//private slots:
//    void checkEditZero(QString dud);

public slots:
    void updateTemTextBoxes();
    void updateChromaticTextBoxes();
    void updateSourceSizeTextBoxes();
    void updateTextBoxes() {updateTemTextBoxes(); updateChromaticTextBoxes(); updateSourceSizeTextBoxes();}

    void updateTemManager();
    void updateChromaticManager();
    void updateSourceSizeManager();
    void updateManager() {updateTemManager(); updateChromaticManager(); updateSourceSizeManager();}

    void setModeStyles(SimulationMode md, bool tem_image);

private:
    Ui::IncoherenceFrame *ui;

    MainWindow* Main;
};

#endif // INCOHERENCEFRAME_H
