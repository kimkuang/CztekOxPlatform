#include "UNumberLabel.h"
#include <QDebug>
#define ENNABLE_CHANGE_CHANNEL 0

UNumberLabel::UNumberLabel(int number, int maxNumber, QWidget *parent) :
    QLabel(parent),
    m_maxNumber(maxNumber)
{
    SetNumber(number);
}

void UNumberLabel::SetNumber(int number)
{
    m_number = number;
    setFrameShape(QFrame::Box);
    setAlignment(Qt::AlignCenter);
    setStyleSheet("background-color: rgb(128, 128, 128); color: rgb(0, 0, 255); font-size: 20pt; font-weight: bold");
    setText(QChar('A' + m_number));
}

void UNumberLabel::mousePressEvent(QMouseEvent *ev)
{
    qDebug()<<ev->button();
#if ENNABLE_CHANGE_CHANNEL
    if (ev->button() == Qt::LeftButton) {
        int num = m_number + 1;
        if (num >= m_maxNumber)
            num = 0;
        SetNumber(num);
    }
#endif
}
