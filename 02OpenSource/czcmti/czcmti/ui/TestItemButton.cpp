#include "TestItemButton.h"

TestItemButton::TestItemButton(const T_ImageTestItem &imgTestItem, QWidget *parent) :
    QPushButton(parent),
    m_imgTestItem(imgTestItem)
{
    ResetButton();
}

TestItemButton::~TestItemButton()
{
}

T_ImageTestItem TestItemButton::GetImageTestItem() const
{
    return m_imgTestItem;
}

void TestItemButton::ResetButton()
{
    this->setText(m_imgTestItem.InstanceName);
    this->setStyleSheet("background-color: rgb(240, 240, 240);color: rgb(0, 0, 0)");
}
