#ifndef TESTITEMBUTTON_H
#define TESTITEMBUTTON_H
#include <QPushButton>
#include "service/TestItemService.h"

class TestItemButton : public QPushButton
{
    Q_OBJECT
public:
    explicit TestItemButton(const T_ImageTestItem &imgTestItem, QWidget *parent = Q_NULLPTR);
    ~TestItemButton();

    T_ImageTestItem GetImageTestItem() const;
    void ResetButton();

private:
    T_ImageTestItem m_imgTestItem;
};

#endif // TESTITEMBUTTON_H
