//
// Created by jon on 02/06/18.
//

#ifndef CLTEM_FLATTITLEBAR_H
#define CLTEM_FLATTITLEBAR_H

#include <QWidget>
#include <QMouseEvent>

// all taken from: https://stackoverflow.com/questions/43319368/i-want-to-create-a-custom-title-bar-in-qt

class FlatTitleBar : public QWidget {
private:
    QWidget *m_parent;
    QPoint m_pCursor;

public:
    FlatTitleBar( QWidget *parent) ;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};


#endif //CLTEM_FLATTITLEBAR_H
