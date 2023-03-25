//
// Created by jon on 21/04/18.
//

#ifndef CLTEM_GUIUTILS_H
#define CLTEM_GUIUTILS_H


#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtGui/QColor>

#include <vector>
#include <unordered_map>

namespace GuiUtils {

    extern const unsigned int edit_precision;

    // https://stackoverflow.com/questions/3490336/how-to-reveal-in-finder-or-show-in-explorer-with-qt
    void openInDefault(const QString &pathIn);

    ///
    /// This is where atom colours are defined for the structure plotting
    ///

    extern std::unordered_map<int, std::string> MapNumberToColour;
    extern const std::vector<std::pair<int, std::string>> VectorNumberToColour;

    // Getter for the map. Replicated convenience of [] that can't be used as the map
    // is const (and [] will add elements if the key is not present)
    std::string ElementNumberToColour(int A);
    QColor ElementNumberToQColour(int A);

    int ConstructElementColourMap();

    extern int dummy_construct_map;

};


#endif //CLTEM_GUIUTILS_H
