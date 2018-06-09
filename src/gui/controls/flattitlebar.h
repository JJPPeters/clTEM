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
private slots:

    void minimise_window();
    void maximise_window();

public:
    explicit FlatTitleBar( QWidget *parent = nullptr) ;

    bool testHitButtonsGlobal(long x, long y);
//    bool nativeEvent(const QByteArray& eventType, void *message, long *result);
    bool testHitButtonGlobal(QString name, long x, long y);

protected:
//    void mousePressEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);
};


#endif //CLTEM_FLATTITLEBAR_H
