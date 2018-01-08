#ifndef TABPANELBAR_H
#define TABPANELBAR_H

#include <QWidget>
#include <QtWidgets/QTabBar>

class tabPanelBar : public QTabBar
{
    Q_OBJECT

public:
    explicit tabPanelBar(QWidget *parent = 0);

    QSize sizeHint() const Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
};

#endif // TABPANELBAR_H