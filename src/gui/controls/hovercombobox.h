//
// Created by jonat on 20/01/2019.
//

#ifndef CLTEM_HOVERCOMBOBOX_H
#define CLTEM_HOVERCOMBOBOX_H


#include <QtWidgets/QComboBox>
#include <QtWidgets/QStylePainter>
#include <iostream>

// This entire class is just to get around a quirk of the 'hover' stylesheet still being applied after electing an item
// from the dropdown
class HoverComboBox : public QComboBox {

    Q_OBJECT;

private slots:
    bool testCursorInside() {
        return rect().contains(mapFromGlobal(QCursor::pos()));
    }

public:
    explicit HoverComboBox(QWidget *parent = nullptr): QComboBox(parent) {}

    // https://stackoverflow.com/questions/51963222/programmatically-applying-the-qt-mouse-hover-button-highlighting-on-any-button
    void paintEvent(QPaintEvent *pe) override {
        QStylePainter p(this);

        QStyleOptionComboBox option;
        initStyleOption(&option);
        if(!testCursorInside()) // need to force this
            option.state &= ~QStyle::State_MouseOver;

        p.drawComplexControl(QStyle::CC_ComboBox, option);
        p.drawControl(QStyle::CE_ComboBoxLabel, option);
    }
};


#endif //CLTEM_HOVERCOMBOBOX_H
