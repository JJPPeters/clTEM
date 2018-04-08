#include "globalsettingsframe.h"
#include "ui_globalsettingsframe.h"

GlobalSettingsFrame::GlobalSettingsFrame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GlobalSettingsFrame)
{
    ui->setupUi(this);
}

GlobalSettingsFrame::~GlobalSettingsFrame()
{
    delete ui;
}
