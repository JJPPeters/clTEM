#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <QtWidgets/QMessageBox>
#include <clmanager.h>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <controls/borderlesswindow.h>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
//    MainWindow w;

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

    MainWindow w;

    QFile f(":/Theme/flat-theme.qss");
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&f);
        QString s = in.readAll();
        f.close();

        QString d1 = "#2A2A2A"; // darkest
        QString d2 = "#404040"; // dark
        QString d3 = "#303030"; // lighter than darkest (acts as a 'clicked' for d1)
        QString l1 = "#D8D8D8"; // lightest
        QString l2 = "#777777"; // light

        QString a1 = "#6A9D1A"; // accent
        QString c1 = "#9D1A29"; // close/negative
        QString c2 = "#c52033"; // close/negative lighter (acts as a 'clicked' for c1)


        s.replace("{d1}", d1);
        s.replace("{d2}", d2);
        s.replace("{d3}", d3);
        s.replace("{l1}", l1);
        s.replace("{l2}", l2);
        s.replace("{a1}", a1);
        s.replace("{c1}", c1);
        s.replace("{c2}", c2);

        a.setStyleSheet(s);

        // this is so the plots function properly...
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(d1));
        darkPalette.setColor(QPalette::Mid, QColor(l2));

        qApp->setPalette(darkPalette);
    }

    w.show();

    return a.exec();
}
