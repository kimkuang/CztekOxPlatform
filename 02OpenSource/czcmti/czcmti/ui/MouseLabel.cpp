#include "mouselabel.h"
#include <QDebug>

MouseLabel::MouseLabel(QWidget *parent) :
    QLabel(parent)
{

}

MouseLabel::~MouseLabel()
{
}

QSize MouseLabel::sizeHint() const
{
    return QSize(50, 20);
}

bool MouseLabel::event(QEvent * event)
{
    QEvent::Type eventType = event->type();
//    if (eventType == QEvent::HoverEnter) {
//        qDebug();
//        this->setStyleSheet("color: blue");
//        return true;
//    }
//    else if (eventType == QEvent::HoverLeave) {
//        qDebug();
//        this->setStyleSheet("color: red;");
//        return true;
//    }
    if (eventType == QEvent::Enter) {
        this->setStyleSheet("color: blue");
        return true;
    }
    else if (eventType == QEvent::Leave) {
        this->setStyleSheet("color: red;");
        return true;
    }
    return QLabel::event(event);
}

void MouseLabel::mouseDoubleClickEvent(QMouseEvent * event)
{
    Q_UNUSED(event);
    emit mouseDoubleClicked();
}


