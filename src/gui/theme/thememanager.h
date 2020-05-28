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
#include <QScreen>

struct ThemeManager {

public:
    enum Theme {
        Native,
        Dark,
        DarkGrey
    };

    static Theme CurrentTheme;

    static void setTheme(std::string th) {
        if (th == "Dark") {
            setTheme(Theme::Dark);
        } else if (th == "Dark-grey") {
            setTheme(Theme::DarkGrey);
        } else {
            setTheme(Theme::Native);
        }
    }

    static void setTheme(Theme th) {
        CurrentTheme = th;
        if (th == Theme::Dark) {
            setDarkTheme();
        } else if (th == Theme::DarkGrey) {
            setDarkGreyTheme();
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
        } else if (th == Theme::DarkGrey) {
            settings.setValue("theme", "Dark-grey");
        } else {
            settings.setValue("theme", "Native");
        }
    }

private:
    static void setFontSize() {
        // Some of the HiSPI stuff seems to work only if I set the font?
        QFont font = QGuiApplication::font();
        double font_size = 11;
        font.setPixelSize((int) font_size);
        qApp->setFont(font);
    }

    static void setNativeTheme() {
        setFontSize();

        // it works better if this is before the stylesheet part
        // (sometimes the palette didn't update until a second apply...)
        qApp->setPalette(QApplication::style()->standardPalette());

        QFile f(":/Theme/default-theme.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&f);
            QString s = in.readAll();
            f.close();

            qApp->setStyleSheet(s);
        }
    }

    static void setDarkTheme() {
        setFontSize();

        QFile f(":/Theme/flat-theme.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&f);
            QString s = in.readAll();
            f.close();

            QString t = "dark"; // theme name (for icons)
            QString d1 = "#30343F"; // darkest
            QString d2 = "#3F4551"; // dark
            QString l1 = "#D8D8D8"; // lightest
            QString l2 = "#576070"; // light
            QString e1 = "#3A3F4B"; // disabled/inactive colour

            QString a1 = "#689324"; // accent
            QString a2 = "#7db81e"; // accent (hover)
            QString a3 = "#9ac653"; // accent (clicked)
            QString c1 = "#9D1A29"; // close/negative
            QString c2 = "#c52033"; // close/negative lighter (acts as a 'clicked' for c1)

            // this is so the plots function properly...
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor(d1));
            darkPalette.setColor(QPalette::Mid, QColor(l2));

            // this is for our 'inactive but still usable' text boxes
            darkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(e1));

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

            QFile f_default(":/Theme/default-theme.qss");
            if (f_default.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in_default(&f_default);
                QString s_default = in_default.readAll();
                f_default.close();

                s.replace("{default_settings}", s_default);
            } else
                s.replace("{default_settings}", "");

            qApp->setStyleSheet(s);
        }

    }

    static void setDarkGreyTheme() {
        setFontSize();

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
            QString e1 = "#4E4E4E"; // disabled/inactive colour

            QString a1 = "#689324"; // accent
            QString a2 = "#7db81e"; // accent (hover)
            QString a3 = "#9ac653"; // accent (clicked)
            QString c1 = "#9D1A29"; // close/negative
            QString c2 = "#c52033"; // close/negative lighter (acts as a 'clicked' for c1)

            // this is so the plots function properly...
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor(d1));
            darkPalette.setColor(QPalette::Mid, QColor(l2));

            // this is for our 'inactive but still usable' text boxes
            darkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(e1));

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

            QFile f_default(":/Theme/default-theme.qss");
            if (f_default.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in_default(&f_default);
                QString s_default = in_default.readAll();
                f_default.close();

                s.replace("{default_settings}", s_default);
            } else
                s.replace("{default_settings}", "");

            qApp->setStyleSheet(s);
        }

    }

};


#endif //CLTEM_THEMEMANAGER_H
