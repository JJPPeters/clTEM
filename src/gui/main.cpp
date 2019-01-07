#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <QtWidgets/QMessageBox>
#include <clmanager.h>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <controls/borderlesswindow.h>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include "easylogging++.h"

#ifdef _WIN32
    #include <theme/thememanager.h>
#endif

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
    // Create our application
    QApplication a(argc, argv);

    // set the app details so we can save/load settings (this is critical for using the QStandardPaths)
    QCoreApplication::setOrganizationName("PetersSoft");
    QCoreApplication::setApplicationName("clTEM");

    // Set up logging

    // Get a writable location to save the log file
    auto log_dir = QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() + QString("logs")) + QDir::separator() + "log.log";

    // create a logger for our gui
    el::Loggers::getLogger("gui");

    // create the conf to actually use of config
    el::Configurations defaultConf;
    defaultConf.setToDefault();

    defaultConf.setGlobally(el::ConfigurationType::Filename, log_dir.toStdString());
    defaultConf.setGlobally(el::ConfigurationType::Format, "[%logger] %datetime (Thread:%thread) %level - %msg");

    // set teh config for the loggers
    el::Loggers::reconfigureLogger("default", defaultConf);
    el::Loggers::reconfigureLogger("gui", defaultConf);

    // this makes '%thread' show this string instead of a largely useless number
    el::Helpers::setThreadName("main-gui");

    CLOG(INFO, "gui") << "Logging set up and ready to go!";

    // this tests for opencl, if we dont have it, then there is no point in loading the program
    try {
        ClManager::getDeviceList();
    } catch (const std::exception& e) {
        // TODO: later, this shouldn't exit but should just disable everything relevant
        CLOG(FATAL, "gui") << "Could not find OpenCL.";
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
