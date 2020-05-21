//
// Created by jon on 21/04/18.
//

#ifndef CLTEM_NTHUNITSBOX_H
#define CLTEM_NTHUNITSBOX_H

#include "controls/editunitsbox.h"

class NthUnitsBox : public EditUnitsBox
{

Q_OBJECT

public:
    explicit NthUnitsBox(QWidget *parent = nullptr);

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

    void setText(const QString& s)
    {
        // change units based on what the number is
        // only really works for ints?
        if (s.isEmpty())
            units = " th";
        else {
            int n = s.size() - 1;
            QChar last = s[n];

            if (s == "11" || s == "12" || s == "13")
                units = " th";
            else if (last == '1')
                units = " st";
            else if (last == '2')
                units = " nd";
            else if (last == '3')
                units = " rd";
            else
                units = " th";
        }

        EditUnitsBox::setText(s);

    }

private:
    using EditUnitsBox::setUnits;
};


#endif //CLTEM_NTHUNITSBOX_H
