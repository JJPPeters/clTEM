//
// Created by jon on 02/06/18.
//

#ifndef CLTEM_FLATTITLEBAR_H
#define CLTEM_FLATTITLEBAR_H

#include <QWidget>
#include <QMouseEvent>
#include <QtWidgets/QFrame>

// all taken from: https://stackoverflow.com/questions/43319368/i-want-to-create-a-custom-title-bar-in-qt

class FlatTitleBar : public QFrame {
    Q_OBJECT

    friend class BorderlessWindow;

private slots:

    void minimise_window();
    void maximise_window();

public:
    void setMaximiseIcon();

public:
    explicit FlatTitleBar( QWidget *parent = nullptr) ;

    bool testHitButtonsGlobal(long x, long y);

    bool testHitButtonGlobal(QString name, long x, long y);

    void setTitle(const QString& title);
};


#endif //CLTEM_FLATTITLEBAR_H
