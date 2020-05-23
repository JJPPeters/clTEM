#include "openclframe.h"
#include "ui_openclframe.h"

#include <QRegExpValidator>
#include <QTableWidgetItem>
#include <utilities/commonstructs.h>
#include <dialogs/settings/settingsdialog.h>
#include <kernels.h>

#include "utilities/stringutils.h"

#include "clwrapper.h"

// TODO: construct from list of devices/platforms and occupy the list

OpenClFrame::OpenClFrame(QWidget *parent, std::vector<clDevice>& current_devices) :
    QWidget(parent),
    chosenDevs(current_devices),
    ui(new Ui::OpenClFrame)
{
    ui->setupUi(this);

    ui->tblDevices->setColumnWidth(0, 35);
    ui->tblDevices->setColumnWidth(1, 190);
    ui->tblDevices->setColumnWidth(2, 35);
    ui->tblDevices->setColumnWidth(3, 190);

    ui->tblDevices->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    auto parent_dlg = dynamic_cast<OpenClDialog*>(parentWidget());
    connect(parent_dlg, &OpenClDialog::okSignal, this, &OpenClFrame::dlgOk_clicked);
    connect(parent_dlg, &OpenClDialog::cancelSignal, this, &OpenClFrame::dlgCancel_clicked);
    connect(parent_dlg, &OpenClDialog::applySignal, this, &OpenClFrame::dlgApply_clicked);

    for (const auto &current_device : current_devices) {
        addDeviceToList(current_device); //TODO: implement performance factors
    }

    // doing this will activate the currentIndexChanged action on the cmbPlatform
    // so we that will be dealt with automatically
    populatePlatformCombo();

    QSettings settings;
    settings.beginGroup("opencl/opts");

    bool mad = settings.value("mad").toBool();
    bool no_signed = settings.value("no_signed").toBool();
    bool unsafe_maths = settings.value("unsafe_maths").toBool();
    bool finite_maths = settings.value("finite_maths").toBool();
//    bool native_maths = settings.value("").toBool();

    ui->chkMad->setChecked(mad);
    ui->chkSignedZero->setChecked(no_signed);
    ui->chkUnsafeMaths->setChecked(unsafe_maths);
    ui->chkFiniteMaths->setChecked(finite_maths);
//    ui->chkNativeFuncs->setChecked();
}

OpenClFrame::~OpenClFrame()
{
    delete ui;
}

void OpenClFrame::on_cmbPlatform_currentIndexChanged(int index)
{
    populateDeviceCombo();
}

void OpenClFrame::on_btnAdd_clicked()
{
    if(!ui->cmbDevice->isEnabled())
        return;

    // get strings from combo box
    int platNum = ui->cmbPlatform->currentData().toInt();
    int devNum = ui->cmbDevice->currentData().toInt();

    clDevice dev;

    // instead of trying to intepret the combo string, jsut go through the list again
    for (clDevice d : Devices)
        if (platNum == d.GetPlatformNumber() && devNum == d.GetDeviceNumber())
            dev = d;

    addDeviceToList(dev);

    populateDeviceCombo();
}

void OpenClFrame::addDeviceToList(clDevice dev)
{
    int n = ui->tblDevices->rowCount();
    ui->tblDevices->insertRow(n);

    auto * cell_0 = new QTableWidgetItem();
    cell_0->setTextAlignment(Qt::AlignCenter);
    cell_0->setText(QString::fromStdString(Utils::numToString(dev.GetPlatformNumber())));

    auto cell_1 = cell_0->clone();
    cell_1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    cell_1->setText(QString::fromStdString(dev.GetPlatformName()));

    auto cell_2 = cell_0->clone();
    cell_2->setText(QString::fromStdString(Utils::numToString(dev.GetDeviceNumber())));

    auto cell_3 = cell_1->clone();
    cell_3->setText(QString::fromStdString(dev.GetDeviceName()));

    ui->tblDevices->setItem(n, 0, cell_0);
    ui->tblDevices->setItem(n, 1, cell_1);
    ui->tblDevices->setItem(n, 2, cell_2);
    ui->tblDevices->setItem(n, 3, cell_3);
}

void OpenClFrame::on_btnDelete_clicked()
{
    QList<QTableWidgetItem *> selection = ui->tblDevices->selectedItems();

    std::vector<int> toRemove;
    for(int i = 0; i < selection.size(); ++i)
        if(selection.at(i)->column() == 0)
            toRemove.push_back(selection.at(i)->row());

    std::sort(toRemove.begin(), toRemove.end());

    int n = 0;
    for(int i = 0; i < toRemove.size(); ++i) {
        ui->tblDevices->removeRow(toRemove[i] - n);
        ++n;
    }

    populateDeviceCombo();
}

void OpenClFrame::populatePlatformCombo()
{
    ui->cmbPlatform->clear();
    // get list of opencl Devices
    Devices = OpenCL::GetDeviceList();

    // Handles platforms
    for (clDevice d : Devices)
    {
        int platNum = d.GetPlatformNumber();
        std::string platName = d.GetPlatformName();
        const std::string platformString = Utils::numToString(platNum) + ": " + platName;

        // add the platform if not already present
        // this works as long as the devices are read in order of platform...
        if(platNum+1 > ui->cmbPlatform->count())
            ui->cmbPlatform->addItem(QString::fromStdString(platformString), platNum);
    }
}

void OpenClFrame::populateDeviceCombo()
{
    ui->cmbDevice->clear();
    // get lists of all info from the table (platform ID and device ID)
    int nRows = ui->tblDevices->rowCount();
    std::vector<int> usedPlats(nRows);
    std::vector<int> usedDevs(nRows);

    // these are a bit lengthy, but we just get the QTableWidgetItem at the required row and col,
    // then we get its text, make it a std::string and then convert to an integer....
    // phew
    for (int i = 0; i < nRows; ++i)
    {
        usedPlats[i] = std::stoi(ui->tblDevices->item(i, 0)->text().toStdString());
        usedDevs[i] = std::stoi(ui->tblDevices->item(i, 2)->text().toStdString());
    }

    // Handle devices
    int currentPlat = ui->cmbPlatform->currentData().toInt();
    for (clDevice d : Devices)
    {
        if (d.GetPlatformNumber() == currentPlat)
        {
            int devNum = d.GetDeviceNumber();

            // here we test if this device has been used
            bool isUsed = false;
            for (int i = 0; i < nRows && !isUsed; ++i)
                isUsed = currentPlat == usedPlats[i] && devNum == usedDevs[i];

            if (!isUsed)
            {
                std::string devName = d.GetDeviceName();
                const std::string deviceString = Utils::numToString(devNum) + ": " + devName;

                ui->cmbDevice->addItem(QString::fromStdString(deviceString), devNum);
            }
        }
    }

    ui->cmbDevice->setEnabled(ui->cmbDevice->count() >= 1);
}

void OpenClFrame::dlgCancel_clicked()
{
    // don't need to do anything, just return
    parentWidget()->close();
}

void OpenClFrame::dlgOk_clicked()
{
    // same as clicking apply then closing the dialog
    dlgApply_clicked();
    parentWidget()->close();
}

void OpenClFrame::dlgApply_clicked()
{
    chosenDevs.clear();
    int nRows = ui->tblDevices->rowCount();
    chosenDevs.resize(nRows);

    for (int i = 0; i < nRows; ++i)
    {
        int p = std::stoi(ui->tblDevices->item(i, 0)->text().toStdString());
        int d = std::stoi(ui->tblDevices->item(i, 2)->text().toStdString());

        clDevice dev;
        for (auto dv : Devices)
            if (dv.GetPlatformNumber() == p && dv.GetDeviceNumber() == d)
                dev = dv;

        chosenDevs[i] = dev;
    }

    bool mad = ui->chkMad->isChecked();
    bool no_signed = ui->chkSignedZero->isChecked();
    bool unsafe_maths = ui->chkUnsafeMaths->isChecked();
    bool finite_maths = ui->chkFiniteMaths->isChecked();
    bool native_maths = ui->chkNativeFuncs->isChecked();

    KernelSource::setOptions(mad, no_signed, unsafe_maths, finite_maths);

    // remove all current device entries in the settings and reset them
    QSettings settings;
    settings.remove("opencl");

    settings.setValue("opencl/opts/mad", mad);
    settings.setValue("opencl/opts/no_signed", no_signed);
    settings.setValue("opencl/opts/unsafe_maths", unsafe_maths);
    settings.setValue("opencl/opts/finite_maths", finite_maths);
//    settings.setValue("opencl/opts/native_maths", native_maths);

    int counter = 0;
    for (auto& dev : chosenDevs)
    {
        settings.setValue("opencl/" + QString::number(counter) + "/platform", dev.GetPlatformNumber());
        settings.setValue("opencl/" + QString::number(counter) + "/device", dev.GetDeviceNumber());
        settings.setValue("opencl/" + QString::number(counter) + "/platform_name", QString::fromStdString(dev.GetPlatformName()));
        settings.setValue("opencl/" + QString::number(counter) + "/device_name", QString::fromStdString(dev.GetDeviceName()));
        ++counter;
    }


}