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
#include <QColor>
#include <qrgb>

#include <windows.h>
#include <controls/borderlesswindow.h>
#include <controls/borderlessdialog.h>
#include <QtCore/QSettings>
#include <QScreen>
#include <utility>

struct ThemeColours {
    explicit ThemeColours(QString _icons="dark",
                          const QString& _background="#000000",
                          const QString& _panels="#000000",
                          const QString& _dividers="#000000",
                          const QString& _text="#000000",
                          const QString& _button_text="#000000",
                          const QString& _textbox="#000000",
                          const QString& _inactive="#000000",
                          const QString& _text_inactive="#000000",
                          const QString& _accent="#689324",
                          const QString& _accent_hover="#8EAE5B",
                          const QString& _accent_activated="#A5BF7C",
                          const QString& _close_hover="#9D1A29",
                          const QString& _close_acivated="#c52033") {
        icons = std::move(_icons);

        background = QColor(_background);
        panels = QColor(_panels);
        dividers = QColor(_dividers);
        text = QColor(_text);
        button_text = QColor(_button_text);
        textbox = QColor(_textbox);

        inactive = QColor(_inactive);
        text_inactive = QColor(_text_inactive);

        accent = QColor(_accent);
        accent_hover = QColor(_accent_hover);
        accent_activated = QColor(_accent_activated);

        close_hover = QColor(_close_hover);
        close_acivated = QColor(_close_acivated);
    }

    QString icons;

    QColor background;
    QColor panels;
    QColor dividers;
    QColor text;
    QColor button_text;
    QColor textbox;

    QColor inactive;
    QColor text_inactive;

    QColor accent;
    QColor accent_hover;
    QColor accent_activated;

    QColor close_hover;
    QColor close_acivated;
};

struct ThemeManager {

public:
    enum Theme {
        Native,
        Dark,
        MidDark,
        MidLight,
        Light,
        DarkGrey,
        MidDarkGrey,
        MidLightGrey,
        LightGrey,
        _dummy_last
    };

    static Theme CurrentTheme;

    static ThemeColours DarkTheme;
    static ThemeColours MidDarkTheme;
    static ThemeColours MidLightTheme;
    static ThemeColours LightTheme;

    static std::vector<QString> getThemeNameList() {
        std::vector<QString> out;
        for (int i = 0; i != Theme::_dummy_last; ++i)
            out.emplace_back(themeEnumtoQString(static_cast<Theme>(i)));
        return out;
    }

    static Theme themeQStringtoEnum(QString name) {
        return themeStringtoEnum(name.toStdString());
    }
    static Theme themeStringtoEnum(std::string name) {
        if (name == "Dark") {
            return Theme::Dark;
        } else if (name == "Mid-dark") {
            return Theme::MidDark;
        } else if (name == "Mid-light") {
            return Theme::MidLight;
        } else if (name == "Light") {
            return Theme::Light;
        }else if (name == "Dark-grey") {
            return Theme::DarkGrey;
        } else if (name == "Mid-dark-grey") {
            return Theme::MidDarkGrey;
        } else if (name == "Mid-light-grey") {
            return Theme::MidLightGrey;
        } else if (name == "Light-grey") {
            return Theme::LightGrey;
        } else {
            return Theme::Native;
        }
    }

    static QString themeEnumtoQString(Theme name) {
        return QString::fromStdString(themeEnumtoString(name));
    }
    static std::string themeEnumtoString(Theme name) {
        if (name == Theme::Dark) {
            return "Dark";
        } else if (name == Theme::MidDark) {
            return "Mid-dark";
        } else if (name == Theme::MidLight) {
            return "Mid-light";
        } else if (name == Theme::Light) {
            return "Light";
        }else if (name == Theme::DarkGrey) {
            return "Dark-grey";
        } else if (name == Theme::MidDarkGrey) {
            return "Mid-dark-grey";
        } else if (name == Theme::MidLightGrey) {
            return "Mid-light-grey";
        } else if (name == Theme::LightGrey) {
            return "Light-grey";
        } else {
            return "Native";
        }
    }

    static void setTheme(const std::string& th) {
        setTheme(themeStringtoEnum(th));
    }

    static void setTheme(Theme th) {
        CurrentTheme = th;
        if (th == Theme::Dark) {
            setFlatTheme(DarkTheme);
        } else if (th == Theme::MidDark) {
            setFlatTheme(MidDarkTheme);
        } else if (th == Theme::MidLight) {
            setFlatTheme(MidLightTheme);
        } else if (th == Theme::Light) {
            setFlatTheme(LightTheme);
        } else if (th == Theme::DarkGrey) {
            setFlatTheme(DarkTheme, true);
        } else if (th == Theme::MidDarkGrey) {
            setFlatTheme(MidDarkTheme, true);
        } else if (th == Theme::MidLightGrey) {
            setFlatTheme(MidLightTheme, true);
        } else if (th == Theme::LightGrey) {
            setFlatTheme(LightTheme, true);
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
        settings.setValue("theme", themeEnumtoQString(th));
    }

private:
    static QColor colourToGrey(QColor c, bool v=false) {
        if (v) {
            int g = qGray(c.rgb());
            return QColor(g, g, g);
        } else
            return c;
    }

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

    static void setFlatTheme(ThemeColours thm, bool is_grey=false) {
        setFontSize();

        QFile f(":/Theme/flat-theme.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&f);
            QString s = in.readAll();
            f.close();

            // this is so the plots function properly...
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, colourToGrey(thm.textbox, is_grey));
            darkPalette.setColor(QPalette::Mid, colourToGrey(thm.dividers, is_grey));

            // this is for our 'inactive but still usable' text boxes
            darkPalette.setColor(QPalette::Disabled, QPalette::Text, colourToGrey(thm.text, is_grey));
            darkPalette.setColor(QPalette::Disabled, QPalette::Base, colourToGrey(thm.inactive, is_grey));

            qApp->setPalette(darkPalette);

            s.replace("{t}", thm.icons);

            s.replace("{d1}", colourToGrey(thm.background, is_grey).name());
            s.replace("{d2}", colourToGrey(thm.panels, is_grey).name());
            s.replace("{l1}", colourToGrey(thm.text, is_grey).name());
            s.replace("{l2}", colourToGrey(thm.dividers, is_grey).name());
            s.replace("{g1}", colourToGrey(thm.dividers, is_grey).name());
            s.replace("{t1}", colourToGrey(thm.textbox, is_grey).name());
            s.replace("{i1}", colourToGrey(thm.text_inactive, is_grey).name());
            s.replace("{b1}", colourToGrey(thm.button_text, is_grey).name());
            s.replace("{e1}", colourToGrey(thm.inactive, is_grey).name());
            s.replace("{a1}", colourToGrey(thm.accent, false).name());
            s.replace("{a2}", colourToGrey(thm.accent_hover, false).name());
            s.replace("{a3}", colourToGrey(thm.accent_activated, false).name());
            s.replace("{c1}", colourToGrey(thm.close_hover, false).name());
            s.replace("{c2}", colourToGrey(thm.close_acivated, false).name());

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
