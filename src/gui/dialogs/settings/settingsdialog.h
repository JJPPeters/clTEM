#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <dialogs/settings/frames/ctemareaframe.h>
#include <dialogs/settings/frames/stemdetectorframe.h>
#include <dialogs/settings/frames/stemareaframe.h>
#include <dialogs/settings/frames/thermalscatteringframe.h>
#include <dialogs/settings/frames/arealayoutframe.h>
#include <dialogs/settings/frames/globalsimsettingsframe.h>
#include "dialogs/settings/frames/openclframe.h"
#include "dialogs/settings/frames/fullaberrationframe.h"
#include "dialogs/settings/frames/cifcreatorframe.h"
#include "dialogs/settings/frames/generalsettingsframe.h"
#include "dialogs/settings/frames/plasmonsettingsframe.h"

#include <structure/crystalstructure.h>
#include <controls/borderlessdialog.h>

#include <cif/supercell.h>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public BorderlessDialog
{
    Q_OBJECT

signals:
    void okSignal();
    void cancelSignal();
    void applySignal();

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog() override;

    void showApplyButton(bool show);

    void setOkEnabled(bool enabled);

private slots:
    void on_btnCancel_clicked();

    void on_BtnApply_clicked();

    void on_btnOk_clicked();

protected:
    Ui::SettingsDialog *ui;
};




class OpenClDialog : public SettingsDialog
{
    Q_OBJECT
signals:
    void devicesChanged(std::vector<clDevice>);

private:
    OpenClFrame* OClFrame;

public:
    explicit OpenClDialog(QWidget *parent, std::vector<clDevice> &current_devices);
};


class ThemeDialog : public SettingsDialog
{
Q_OBJECT

private:
    GeneralSettingsFrame* tFrame;

public:
    explicit ThemeDialog(QWidget *parent);
};



class GlobalSettingsDialog : public SettingsDialog
{
Q_OBJECT

private:
    GlobalSimSettingsFrame* GeneralFrame;

public:
    explicit GlobalSettingsDialog(QWidget *parent, std::shared_ptr<SimulationManager> simManager);
};



class AberrationsDialog : public SettingsDialog
{
    Q_OBJECT

signals:
    // this signal is picked up by the mainwindow aberrations frame
    void aberrationsChanged();

private:
    FullAberrationFrame* AberrFrame;

private slots:
    // this slot gets the signal from the fullaberrationsframe
    void coreAberrationsChanged();

public:
    explicit AberrationsDialog(QWidget *parent, std::shared_ptr<MicroscopeParameters> params);

};


class ThermalScatteringDialog : public SettingsDialog
{
    Q_OBJECT

signals:
    // this signal is picked up by the mainwindow aberrations frame
    void phononsChanged();

private:
    ThermalScatteringFrame* ThermalFrame;

private slots:
    // this slot gets the signal from the fullaberrationsframe
    void corePhononsChanged();

public:
    explicit ThermalScatteringDialog(QWidget *parent, std::shared_ptr<SimulationManager> simManager);

};


class PlasmonDialog : public SettingsDialog
{
Q_OBJECT

signals:
    // this signal is picked up by the mainwindow aberrations frame
    void plasmonsChanged();

private slots:
    // this slot gets the signal from the fullaberrationsframe
    void corePlasmonsChanged();

private:
    PlasmonSettingsFrame* PlasmonFrame;

public:
    explicit PlasmonDialog(QWidget *parent, std::shared_ptr<SimulationManager> simManager);

};


class SimAreaDialog : public SettingsDialog
{
Q_OBJECT

private:
    AreaLayoutFrame* LayoutFrame;

public:
    explicit SimAreaDialog(QWidget *parent, std::shared_ptr<SimulationManager> simManager);

    AreaLayoutFrame* getFrame() {return LayoutFrame;}

};




class StemDetectorDialog : public SettingsDialog
{
Q_OBJECT

signals:
    // this signal is picked up by the mainwindow aberrations frame
    void detectorsChanged();

private:
    StemDetectorFrame* DetFrame;

private slots:
    // this slot gets the signal from the fullaberrationsframe
    void coreDetectorsChanged();

public:
    explicit StemDetectorDialog(QWidget *parent, std::vector<StemDetector>& dets);

};




class StemAreaDialog : public SettingsDialog
{
Q_OBJECT

signals:
    void stemAreaChanged();

private:
    StemAreaFrame* AreaFrame;

private slots:
    void coreStemAreaChanged();

public:
    explicit StemAreaDialog(QWidget *parent, std::shared_ptr<StemArea> stem, std::shared_ptr<SimulationArea> sim);

};


class CifCreatorDialog : public SettingsDialog
{
Q_OBJECT

private:
    CifCreatorFrame* CifFrame;

public:
    explicit CifCreatorDialog(QWidget *parent, CIF::CIFReader cif, std::shared_ptr<CIF::SuperCellInfo> info);

};

#endif // SETTINGSDIALOG_H
