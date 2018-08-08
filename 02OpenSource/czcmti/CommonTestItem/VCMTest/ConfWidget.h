#ifndef CONFWIDGET_H
#define CONFWIDGET_H

#include <QWidget>
#include <QMenu>
#include "Option.h"
#include <QStandardItemModel>

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

private:
    Ui::ConfWidget *ui;
    Option *m_option;
};

#endif // CONFWIDGET_H
