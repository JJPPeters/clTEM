//
// Created by jon on 21/04/18.
//

#ifndef CLTEM_EDITUNITSBOX_H
#define CLTEM_EDITUNITSBOX_H

#include <QWidget>
#include <QtWidgets/QLineEdit>

class EditUnitsBox : public QLineEdit
{

Q_OBJECT

protected:
    std::string units;

    std::string background_style;

    std::string foreground_style;

    std::string other_styles;

public:
    explicit EditUnitsBox(QWidget *parent = nullptr);

    void setStyleSheet(const QString& style);

    void setBackgroundStyle(std::string style);

    void setForegroundStyle(std::string style);

    void focusInEvent(QFocusEvent* e) override
    {
        auto t = text();
        QLineEdit::setText(t);
        QLineEdit::focusInEvent(e);
    }

    void focusOutEvent(QFocusEvent* e) override
    {
        auto t = QLineEdit::text();
        setText(t);
        QLineEdit::focusOutEvent(e);
    }

    QString text()
    {
        auto t = QLineEdit::text().toStdString();
        auto u = units;

        if (t != u && t.size() > u.size() && t.substr(t.size() - u.size()) == u)
        {
            t = t.substr(0, t.size() - u.size());
        }

        return QString::fromStdString(t);
    }

    void setText(const QString& s)
    {
        auto t = s.toStdString();

        // remove leading zeros, add one back if the string is empty
        t.erase(0, std::min(t.find_first_not_of('0'), t.size()-1));

        std::string test = "";
        if (!t.empty())
            test = t.substr(0,1);

        if (t.empty() || t.substr(0,1) == ".")
            t = "0" + t;

        t += units;

        QLineEdit::setText(QString::fromStdString(t));
    }

    void setUnits(std::string s)
    {
        // want to preserve textChanged emission for value changes
        const QSignalBlocker blocker(this);

        auto orig = text();

        if (!s.empty() && s.substr(0,1) != " ")
            s = " " + s;
        units = s;

        setText(orig);
    }
};


#endif //CLTEM_EDITUNITSBOX_H
