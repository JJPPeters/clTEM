//
// Created by jon on 02/06/18.
//

#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include "flattitlebar.h"

FlatTitleBar::FlatTitleBar(QWidget *parent) : m_parent(parent->window())
{
    auto title = new QLabel(parent->windowTitle());
    auto pPB = new QPushButton ("x");

    auto *layout = new QHBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(pPB);

    connect(pPB, &QPushButton::clicked, m_parent, &QWidget::close);
}

void FlatTitleBar::mousePressEvent(QMouseEvent *event)
{
    // warning: for testing
    // this doesnt work if the title bar is still there, it will skip a bit
    if(event->button() == Qt::LeftButton)
    {
        m_pCursor = event->globalPos() - m_parent->geometry().topLeft();
        event->accept();
    }
}

void FlatTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        m_parent->move(event->globalPos() - m_pCursor);
        event->accept();
    }
}