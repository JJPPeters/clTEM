#ifndef STATUSLAYOUT_H
#define STATUSLAYOUT_H

#include <QWidget>

namespace Ui {
class StatusLayout;
}

class StatusLayout : public QWidget
{
    Q_OBJECT

public:
    explicit StatusLayout(QWidget *parent = 0);
    ~StatusLayout();

    void setSliceProgress(double prog);
    void setTotalProgress(double prog);
    void setFileLabel(const QString &lbl);

private:
    Ui::StatusLayout *ui;
};

#endif // STATUSLAYOUT_H
