#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <QtWidgets/QMessageBox>
#include <clmanager.h>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;

    try {
        ClManager::getDeviceList();
    } catch (const std::exception& e) {
        // TODO: later, this shouldn't exit but should just disable everything relevant
        QMessageBox msgBox(&w);
        msgBox.setText("Error:");
        msgBox.setInformativeText("Failed to find OpenCl");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setMinimumSize(160, 125);
        msgBox.exec();
        a.exit(1); // not sure I need both of these, but just to be sure
        return 1;
    }

    w.show();

    return a.exec();
}
