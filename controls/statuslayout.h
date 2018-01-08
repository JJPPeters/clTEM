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

    void setSliceProgress(float prog);
    void setTotalProgress(float prog);

private:
    Ui::StatusLayout *ui;
};

#endif // STATUSLAYOUT_H
