#include "mousewidget.h"

MouseWidget::MouseWidget(QWidget *parent) : QWidget(parent)
{

}

MouseWidget::~MouseWidget()
{

}

void MouseWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_UNUSED(event);
    emit mouseDoubleClicked();
//    event->accept();
}

