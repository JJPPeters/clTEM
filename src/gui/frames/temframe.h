#ifndef TEMFRAME_H
#define TEMFRAME_H

#include <QWidget>
#include <simulationmanager.h>

namespace Ui {
class TemFrame;
}

class TemFrame : public QWidget
{
    Q_OBJECT

signals:
    void setCtemCrop(bool);
    void setCtemImage(bool);

public:
    explicit TemFrame(QWidget *parent = 0);
    ~TemFrame();

    void setCropCheck(bool state);
    void setSimImageCheck(bool state);
    void setCcdIndex(int index);
    void setBinningIndex(int index);
    void setDose(double dose);

    int getBinning();
    std::string getCcd();
    bool getSimImage();
    double getDose();

    void populateCcdCombo(std::vector<std::string> names);

    void update_ccd_boxes(std::shared_ptr<SimulationManager> sm);

private slots:
    void on_edtDose_textChanged(const QString &arg1);

    void on_chkCrop_toggled(bool state);

    void on_chkSimImage_toggled(bool state);

private:
    Ui::TemFrame *ui;

};

#endif // TEMFRAME_H
