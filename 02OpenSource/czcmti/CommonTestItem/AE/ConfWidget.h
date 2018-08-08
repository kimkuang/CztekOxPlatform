#ifndef SHADINGCONFWIDGET_H
#define SHADINGCONFWIDGET_H

#include <QWidget>
#include <QMenu>
#include "Option.h"
#include <QStandardItemModel>
#include <QButtonGroup>

namespace Ui {
class ConfWidget;
}

class ConfWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfWidget(Option *option, QWidget *parent = 0);
    ~ConfWidget();

    int RestoreDefaults();
    int Cache2Ui();
    int Ui2Cache();

private slots:
    void CheckBox128PixelClick(void);
    void CheckBoxAvgPixelClick(void);
    void CheckBoxChlPixelClick(void);
    void CheckBoxRememberAEClick(void);

private:
    Ui::ConfWidget *ui;
    Option *m_option;
    QButtonGroup m_btnGroup1;
    QButtonGroup m_btnGroup2;
};

#endif // SHADINGCONFWIDGET_H
