//
// Created by jon on 02/06/18.
//

#ifndef CLTEM_BORDERLESSWINDOW_H
#define CLTEM_BORDERLESSWINDOW_H


#include <QtWidgets/QMainWindow>
#include <QApplication>
#include <QMouseEvent>

// for testing
#include <iostream>

// for the mouse move event filter stuff:
// https://stackoverflow.com/questions/1935021/getting-mousemoveevents-in-qt

class BorderlessWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit BorderlessWindow(QWidget *parent = 0);

private:
    // used to define where our edge is to grab it
    static unsigned int border;

    bool eventFilter(QObject *obj, QEvent *event) override;

    void mouseEdgeEvent(QMouseEvent *event);
};


#endif //CLTEM_BORDERLESSWINDOW_H
