#ifndef SOURCEFRAME_H
#define SOURCEFRAME_H

#include <QWidget>
#include <mainwindow.h>

namespace Ui {
class SourceFrame;
}

class SourceFrame : public QWidget
{
    Q_OBJECT

public:
    explicit SourceFrame(QWidget *parent = 0);

    ~SourceFrame() override;

    void assignMainWindow(MainWindow* m) {
        Main = m;
        setModeStyles(m->Manager->mode(), m->Manager->ctemImageEnabled());
        updateTextBoxes();
    }

    void updateManagerFromGui();

private slots:
    void checkEditZero(QString dud);

    void on_btnMore_clicked();

    void on_edtVoltage_textChanged(const QString &arg1);

public slots:
    void updateTextBoxes();

    void setModeStyles(SimulationMode md, bool tem_image);

private:
    Ui::SourceFrame *ui;

    MainWindow* Main;
};

#endif // SOURCEFRAME_H
