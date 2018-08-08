#ifndef MOUSEWIDGET_H
#define MOUSEWIDGET_H

#include <QWidget>
#include <QMouseEvent>

class MouseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MouseWidget(QWidget *parent = 0);
    ~MouseWidget();

protected:
    void mouseDoubleClickEvent(QMouseEvent * event);

signals:
    void mouseDoubleClicked();
};

#endif // MOUSEWIDGET_H
