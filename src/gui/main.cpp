#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <QtWidgets/QMessageBox>
#include <clmanager.h>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

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

    QFile f(":/Theme/flat-theme.qss");
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&f);
        QString s = in.readAll();
        f.close();

        QString d1 = "#2A2A2A";
        QString l2 = "#777777";

        s.replace("{d2}", "#404040"); // dark
        s.replace("{d1}", d1); // darkest
        s.replace("{l1}", "#D8D8D8"); // lightest
        s.replace("{l2}", l2); // light
        s.replace("{a1}", "#6A9D1A"); // accent

        a.setStyleSheet(s);

        // this is so the plots function properly...
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(d1));
        darkPalette.setColor(QPalette::Mid, QColor(l2));

        qApp->setPalette(darkPalette);

    }

    w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    w.show();

    return a.exec();
}
