#ifndef BOOLFRAME_H
#define BOOLFRAME_H

#include <QFrame>
#include <QHBoxLayout>
#include <QRadioButton>

class BoolFrame : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(QString TrueText READ trueText WRITE setTrueText)
    Q_PROPERTY(QString FalseText READ falseText WRITE setFalseText)
    Q_PROPERTY(bool Value READ value WRITE setValue)
public:
    explicit BoolFrame(QWidget *parent = 0);

    void setTrueText(const QString &text = "True");
    QString trueText() const;
    void setFalseText(const QString &text = "False");
    QString falseText() const;
    void setValue(const bool val);
    bool value() const;

signals:
    void ValueChanged(const bool newVal);

private:
    QHBoxLayout *m_layout;
    QRadioButton *m_rdoTrue;
    QRadioButton *m_rdoFalse;
};

#endif // BOOLFRAME_H
