#ifndef CURRENTCONFWIDGET_H
#define CURRENTCONFWIDGET_H

#include <QWidget>
#include "Option.h"

namespace Ui {
class ConfWidget;
}

class ConfWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfWidget(QWidget *parent = 0);
    ~ConfWidget();

    int RestoreDefaults();
    int Cache2Ui();
    int Ui2Cache();


private:
    Ui::ConfWidget *ui;
    Option *m_option;
};

#endif // CURRENTCONFWIDGET_H
