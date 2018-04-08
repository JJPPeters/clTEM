#ifndef GLOBALSETTINGSFRAME_H
#define GLOBALSETTINGSFRAME_H

#include <QWidget>

namespace Ui {
class GlobalSettingsFrame;
}

class GlobalSettingsFrame : public QWidget
{
    Q_OBJECT

public:
    explicit GlobalSettingsFrame(QWidget *parent = 0);
    ~GlobalSettingsFrame();

private:
    Ui::GlobalSettingsFrame *ui;
};

#endif // GLOBALSETTINGSFRAME_H
