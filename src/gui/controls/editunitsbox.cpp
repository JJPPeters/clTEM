//
// Created by jon on 21/04/18.
//

#include "editunitsbox.h"

#include <utility>

EditUnitsBox::EditUnitsBox(QWidget *parent) : QLineEdit(parent)
{
    units = "";
    background_style = "";
    foreground_style = "";
    other_styles = "";
}

void EditUnitsBox::setStyleSheet(const QString& style) {
    other_styles = style.toStdString();

    std::string full_style = background_style + ";\n\n" + foreground_style + ";\n\n" + other_styles;

    QLineEdit::setStyleSheet(QString::fromStdString(full_style));
}

void EditUnitsBox::setBackgroundStyle(std::string style) {
    background_style = style;
    setStyleSheet(QString::fromStdString(other_styles));
}

void EditUnitsBox::setForegroundStyle(std::string style) {
    foreground_style = style;
    setStyleSheet(QString::fromStdString(other_styles));
}

