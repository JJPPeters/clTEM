#ifndef TEMFRAME_H
#define TEMFRAME_H

#include <QWidget>

namespace Ui {
class TemFrame;
}

class TemFrame : public QWidget
{
    Q_OBJECT

signals:
    void startSim();
    void stopSim();
    void setCtemCrop(bool);

public:
    explicit TemFrame(QWidget *parent = 0);
    ~TemFrame();

    void setActive(bool active);

    void setCrop(bool state);

private slots:
    void on_edtDose_textChanged(const QString &arg1);

    void on_btnExitWave_clicked();

    void on_btnCancel_clicked();

    void on_chkCrop_toggled(bool state);

private:
    Ui::TemFrame *ui;
};

#endif // TEMFRAME_H
