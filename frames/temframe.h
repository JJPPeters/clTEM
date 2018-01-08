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

public:
    explicit TemFrame(QWidget *parent = 0);
    ~TemFrame();

    void setActive(bool active);

private slots:
    void on_edtDose_textChanged(const QString &arg1);

    void on_btnExitWave_clicked();

    void on_btnCancel_clicked();

private:
    Ui::TemFrame *ui;
};

#endif // TEMFRAME_H
