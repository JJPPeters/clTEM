//
// Created by jon on 21/04/18.
//

#ifndef CLTEM_GUIUTILS_H
#define CLTEM_GUIUTILS_H


#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

struct GuiUtils {

    static unsigned int edit_precision;

    // https://stackoverflow.com/questions/3490336/how-to-reveal-in-finder-or-show-in-explorer-with-qt
    static void openInDefault(const QString &pathIn)
    {

        QDesktopServices::openUrl(QUrl(pathIn));
    }

};


#endif //CLTEM_GUIUTILS_H
