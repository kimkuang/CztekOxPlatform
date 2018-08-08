#ifndef MOUSELABEL_H
#define MOUSELABEL_H

#include <QLabel>
#include <QMouseEvent>

class MouseLabel : public QLabel
{
    Q_OBJECT
public:
    MouseLabel(QWidget *parent = 0);
    ~MouseLabel();
    QSize sizeHint() const Q_DECL_OVERRIDE;

signals:
    void mouseDoubleClicked(void);
protected:
    bool event(QEvent * event);
    void mouseDoubleClickEvent(QMouseEvent * event);
};

#endif // MOUSELABEL_H
