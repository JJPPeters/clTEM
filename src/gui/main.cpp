#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <QtWidgets/QMessageBox>
#include <clmanager.h>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <controls/borderlesswindow.h>
#include <theme/thememanager.h>
#include <QtCore/QSettings>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    // this tests for opencl, if we dont have it, then there is no point in loading the program
    try {
        ClManager::getDeviceList();
    } catch (const std::exception& e) {
        // TODO: later, this shouldn't exit but should just disable everything relevant
        QMessageBox msgBox(nullptr);
        msgBox.setText("Error:");
        msgBox.setInformativeText("Failed to find OpenCl");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setMinimumSize(160, 125);
        msgBox.exec();
        a.exit(1); // not sure I need both of these, but just to be sure
        return 1;
    }

    // set the app details so we can save/load settings
    QCoreApplication::setOrganizationName("PetersSoft");
    QCoreApplication::setApplicationName("clTEM");

    MainWindow w;

#ifdef _WIN32

    QSettings settings;
    if (!settings.contains("theme"))
        settings.setValue("theme", "Native");

    auto theme = settings.value("theme").toString().toStdString();

    // load this theme from the settings
    ThemeManager::setTheme(theme);

#endif

    w.show();

    return a.exec();
}
