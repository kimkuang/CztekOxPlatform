#ifndef UNUMBERLABEL_H
#define UNUMBERLABEL_H
#include <QLabel>
#include <QMouseEvent>

class UNumberLabel : public QLabel
{
    Q_OBJECT
public:
    explicit UNumberLabel(int number, int maxNumber, QWidget *parent=Q_NULLPTR);

    void SetNumber(int number);
    inline int GetNumber() const
    {
        return m_number;
    }

protected:
    void mousePressEvent(QMouseEvent *ev);

private:
    int m_number;
    const int m_maxNumber;
};

#endif // UNUMBERLABEL_H
