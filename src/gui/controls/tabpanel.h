#ifndef TABPANEL_H
#define TABPANEL_H

#include <QtWidgets>
#include <QTabWidget>

// Class that implements a tabwidget without the tabbar section
// I mainly use this because I want the background colour the tab pane uses.
class tabPanel : public QTabWidget
{
    Q_OBJECT

public:
    explicit tabPanel(QWidget *parent = 0);

signals:

public slots:
};

#endif // TABPANEL_H
