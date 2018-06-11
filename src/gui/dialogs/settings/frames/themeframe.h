#ifndef GLOBALTHEMEFRAME_H
#define GLOBALTHEMEFRAME_H

#include <QWidget>
#include <simulationmanager.h>
#include <structure/structureparameters.h>


namespace Ui {
class ThemeFrame;
}

class ThemeFrame : public QWidget
{
    Q_OBJECT

private slots:
    void dlgOk_clicked();

    void dlgApply_clicked();

    void dlgCancel_clicked();

    void on_cmbTheme_currentIndexChanged(int index);

public:
    explicit ThemeFrame(QWidget *parent);
    ~ThemeFrame();

private:
    Ui::ThemeFrame *ui;
};

#endif // GLOBALTHEMEFRAME_H
