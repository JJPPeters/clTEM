#ifndef OPENCLFRAME_H
#define OPENCLFRAME_H

#include <QWidget>

#include "clstatic.h"

namespace Ui {
class OpenClFrame;
}

class OpenClFrame : public QWidget
{
    Q_OBJECT

public:
    explicit OpenClFrame(QWidget *parent, std::vector<clDevice>& current_devices);

    ~OpenClFrame();

    std::vector<clDevice> getChosenDevices() {return chosenDevs;}

private slots:
    void on_cmbPlatform_currentIndexChanged(int index);

    void on_btnAdd_clicked();

    void on_btnDelete_clicked();

    void dlgOk_clicked();

    void dlgApply_clicked();

    void dlgCancel_clicked();

private:
    Ui::OpenClFrame *ui;

    std::vector<clDevice> Devices;

    std::vector<clDevice>& chosenDevs;

    void populatePlatformCombo();
    void populateDeviceCombo();

    void addDeviceToList(clDevice dev);
};

#endif // OPENCLFRAME_H
