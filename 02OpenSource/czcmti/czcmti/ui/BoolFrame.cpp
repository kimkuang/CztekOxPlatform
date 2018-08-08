#include "BoolFrame.h"

BoolFrame::BoolFrame(QWidget *parent) :
    QFrame(parent)
{
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Plain);
    m_layout = new QHBoxLayout(this);
    m_rdoTrue = new QRadioButton(this);
    m_rdoFalse = new QRadioButton(this);
    m_layout->addWidget(m_rdoTrue);
    m_layout->addWidget(m_rdoFalse);
    m_layout->setContentsMargins(9, 0, 9, 0);
    setLayout(m_layout);

    setTrueText();
    setFalseText();
    setValue(true);
}

void BoolFrame::setTrueText(const QString &text)
{
    m_rdoTrue->setText(text);
}

QString BoolFrame::trueText() const
{
    return m_rdoTrue->text();
}

void BoolFrame::setFalseText(const QString &text)
{
    m_rdoFalse->setText(text);
}

QString BoolFrame::falseText() const
{
    return m_rdoFalse->text();
}

void BoolFrame::setValue(const bool val)
{
    m_rdoTrue->setChecked(val);
    m_rdoFalse->setChecked(!val);
}

bool BoolFrame::value() const
{
    return m_rdoTrue->isChecked();
}
