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
#include "utilities/logging.h"

#ifdef _WIN32
    #include <theme/thememanager.h>
#endif

int main(int argc, char *argv[]) {
    // Create our application
    QApplication a(argc, argv);


    // set the app details so we can save/load settings (this is critical for using the QStandardPaths)
    QCoreApplication::setOrganizationName("PetersSoft");
    QCoreApplication::setApplicationName("clTEM");

    // Set up logging

    // Get a writable location to save the log file
    auto log_dir = QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + QString("logs")) + QDir::separator() + "log.log";

    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).exists())
        QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));

    // create a logger for our gui and simulation
    el::Loggers::getLogger("gui");
    el::Loggers::getLogger("sim");

    // create the conf to actually use of config
    el::Configurations defaultConf;
    defaultConf.setToDefault();

    defaultConf.setGlobally(el::ConfigurationType::Filename, log_dir.toStdString());
    defaultConf.setGlobally(el::ConfigurationType::Format, "[%logger] %datetime (thread:%thread) %level - %func: %msg");
    defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
    defaultConf.setGlobally(el::ConfigurationType::MaxLogFileSize, "1073741824");

    QSettings settings;
    if (!settings.contains("logging"))
        settings.setValue("logging", false);

    if (settings.value("logging").toBool())
        defaultConf.setGlobally(el::ConfigurationType::ToFile, "true");
    else
        defaultConf.setGlobally(el::ConfigurationType::ToFile, "false");

    defaultConf.set(el::Level::Error, el::ConfigurationType::ToFile, "true");

    // set the config for the loggers
    el::Loggers::reconfigureAllLoggers(defaultConf);

    // this makes '%thread' show this string instead of a largely useless number
    el::Helpers::setThreadName("main-gui");

    // this tests for opencl, if we dont have it, then there is no point in loading the program
    try {
        ClManager::getDeviceList();
    } catch (const std::exception& e) {
        // TODO: later, this shouldn't exit but should just disable everything relevant
        CLOG(ERROR, "gui") << "Could not find OpenCL: " << e.what();
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

    if (!settings.contains("theme"))
        settings.setValue("theme", "Native");

    auto theme = settings.value("theme").toString().toStdString();

    // load this theme from the settings
    ThemeManager::setTheme(theme);

#endif

    w.show();

    return a.exec();
}
