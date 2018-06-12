//
// Created by Jon on 12/06/2018.
//

#ifndef CLTEM_THEMEMANAGER_H
#define CLTEM_THEMEMANAGER_H

#include <QApplication>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QPalette>
#include <QtWidgets/QStyle>
#include <QtWidgets/QWidget>

#include <windows.h>
#include <controls/borderlesswindow.h>
#include <controls/borderlessdialog.h>
#include <QtCore/QSettings>


struct ThemeManager {

public:
    enum Theme {
        Native,
        Dark
    };

    static Theme CurrentTheme;

    static void setTheme(std::string th) {
        if (th == "Dark") {
            setTheme(Theme::Dark);
        } else {
            setTheme(Theme::Native);
        }
    }

    static void setTheme(Theme th) {
        CurrentTheme = th;
        if (th == Theme::Dark) {
            setDarkTheme();
        } else {
            setNativeTheme();
        }

        for (auto &widget: QApplication::topLevelWidgets()) {
            if (widget->isWindow()) {

                auto bw = dynamic_cast<BorderlessWindow*>(widget);
                auto bd = dynamic_cast<BorderlessDialog*>(widget);

                if (bw || bd) {
                    HWND id = (HWND)widget->winId();
                    // this redraws everything (without moving it)
                    SetWindowPos(id, NULL, 0, 0, 0, 0,
                                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                                 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
                }

                if (bw) {
                    bw->setMenuBarVisible(th != Theme::Native);
                    bw->window_borderless();
                } else if (bd) {
                    bd->setMenuBarVisible(th != Theme::Native);
                    bd->window_borderless();
                    // this fixes a problem where the controls would not show everything
                    // I assume because it has a fixed size (and the controls change size)
                    bd->setFixedSize(bd->minimumSizeHint());
                }

            }
        }

        setSettings(th);
    }

    static void setSettings(Theme th) {
        QSettings settings;
        if (th == Theme::Dark) {
            settings.setValue("theme", "Dark");
        } else {
            settings.setValue("theme", "Native");
        }
    }

private:
    static void setNativeTheme() {
        // remove our stylesheet
        qApp->setStyleSheet("");
        // reset our palette
        qApp->setPalette(QApplication::style()->standardPalette());
    }

    static void setDarkTheme() {

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

            qApp->setStyleSheet(s);

            // this is so the plots function properly...
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor(d1));
            darkPalette.setColor(QPalette::Mid, QColor(l2));

            qApp->setPalette(darkPalette);
        }

    }

};


#endif //CLTEM_THEMEMANAGER_H