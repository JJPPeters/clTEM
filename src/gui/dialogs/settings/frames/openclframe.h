#ifndef OPENCLFRAME_H
#define OPENCLFRAME_H

#include <QWidget>

#include "clstatic.h"
#include "clmanager.h"

namespace Ui {
class OpenClFrame;
}

class OpenClFrame : public QWidget
{
    Q_OBJECT

public:
    explicit OpenClFrame(QWidget *parent, std::vector<clDevice> current_devices);

    ~OpenClFrame();

    std::tuple<std::vector<clDevice>, std::vector<float>> getChosenDevices() {return std::make_tuple(chosenDevs, chosenRatios);}

private slots:
    void on_cmbPlatform_currentIndexChanged(int index);

    void on_btnAdd_clicked();

    void on_btnDelete_clicked();

    void dlgOk_clicked();

    void dlgApply_clicked();

    void dlgCancel_clicked();

    void on_edtRatio_textChanged(const QString &arg1);

private:
    Ui::OpenClFrame *ui;

    std::vector<clDevice> Devices;

    std::vector<clDevice> chosenDevs;
    std::vector<float> chosenRatios;

    void populatePlatformCombo();
    void populateDeviceCombo();

    void addDeviceToList(clDevice dev, float ratio);
};

#endif // OPENCLFRAME_H
