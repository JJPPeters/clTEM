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

#include <QtGui/QPixmapCache>
#include <QtWidgets/QComboBox>

struct ThemeManager {

public:
    enum Theme {
        Native,
        Dark,
        Light
    };

    static Theme CurrentTheme;

    static void setTheme(std::string th) {
        if (th == "Dark") {
            setTheme(Theme::Dark);
        } else if (th == "Light") {
            setTheme(Theme::Light);
        } else {
            setTheme(Theme::Native);
        }
    }

    static void setTheme(Theme th) {
        CurrentTheme = th;
        if (th == Theme::Dark) {
            setDarkTheme();
        } else if (th == Theme::Light) {
            setLightTheme();
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
        } else if (th == Theme::Light) {
            settings.setValue("theme", "Light");
        } else {
            settings.setValue("theme", "Native");
        }
    }

public:
    static void setNativeTheme(bool use_light=true) {
        if (!use_light)
            setNativeDarkTheme();
        else
            setNativeLightTheme();
    }

    static void setNativeLightTheme() {
        // remove any stylesheet we may have set (this is needed to remove my custom themes
        qApp->setStyleSheet("");
        // reset our palette to the default
        qApp->setPalette(QApplication::style()->standardPalette());
    }

    static void setNativeDarkTheme() {
        // I tried doing this with QPalette, but it is bullshit and doesnt apply to everything...








        QFile f(":/Theme/native-dark.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&f);
            QString s = in.readAll();
            f.close();

            // this is so the plots function properly...
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor("#191919"));
            darkPalette.setColor(QPalette::Mid, QColor("#2B2B2B"));
            qApp->setPalette(darkPalette);

            qApp->setStyleSheet(s);
        }




    }

    static void setDarkTheme() {
        QFile f(":/Theme/flat-theme.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&f);
            QString s = in.readAll();
            f.close();

            QString t = "dark"; // theme name (for icons)
            QString d1 = "#2A2A2A"; // darkest
            QString d2 = "#404040"; // dark
            QString l1 = "#D8D8D8"; // lightest
            QString l2 = "#777777"; // light
            QString e1 = "#808080"; // disabled/inactive colour

            QString a1 = "#6A9D1A"; // accent
            QString a2 = "#7db81e"; // accent (hover)
            QString a3 = "#9ac653"; // accent (clicked)
            QString c1 = "#9D1A29"; // close/negative
            QString c2 = "#c52033"; // close/negative lighter (acts as a 'clicked' for c1)

            // this is so the plots function properly...
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor(d1));
            darkPalette.setColor(QPalette::Mid, QColor(l2));

            qApp->setPalette(darkPalette);

            s.replace("{t}", t);

            s.replace("{d1}", d1);
            s.replace("{d2}", d2);
            s.replace("{l1}", l1);
            s.replace("{l2}", l2);
            s.replace("{e1}", e1);
            s.replace("{a1}", a1);
            s.replace("{a2}", a2);
            s.replace("{a3}", a3);
            s.replace("{c1}", c1);
            s.replace("{c2}", c2);

            qApp->setStyleSheet(s);
        }

    }

    static void setLightTheme() {
        QFile f(":/Theme/flat-theme.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&f);
            QString s = in.readAll();
            f.close();

            QString t = "light"; // theme name (for icons)
            QString d1 = "#F0F0F0"; // darkest
            QString d2 = "#FFFFFF"; // dark
            QString l1 = "#1a1a1a"; // lightest
            QString l2 = "#9c9c9c"; // light
            QString e1 = "#a1a1a1"; // disabled/inactive colour

            QString a1 = "#6A9D1A"; // accent
            QString a2 = "#7db81e"; // accent (hover)
            QString a3 = "#9ac653"; // accent (clicked)
            QString c1 = "#9D1A29"; // close/negative
            QString c2 = "#c52033"; // close/negative lighter (acts as a 'clicked' for c1)

            // this is so the plots function properly...
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor(d1));
            darkPalette.setColor(QPalette::Mid, QColor(l2));

            qApp->setPalette(darkPalette);

            s.replace("{t}", t);
            s.replace("{d1}", d1);
            s.replace("{d2}", d2);
            s.replace("{l1}", l1);
            s.replace("{l2}", l2);
            s.replace("{e1}", e1);
            s.replace("{a1}", a1);
            s.replace("{a2}", a2);
            s.replace("{a3}", a3);
            s.replace("{c1}", c1);
            s.replace("{c2}", c2);

            qApp->setStyleSheet(s);
        }

    }

};


#endif //CLTEM_THEMEMANAGER_H
