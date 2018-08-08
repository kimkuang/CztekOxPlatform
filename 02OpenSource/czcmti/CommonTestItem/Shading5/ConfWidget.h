#ifndef SHADINGCONFWIDGET_H
#define SHADINGCONFWIDGET_H

#include <QWidget>
#include "Option.h"

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

#endif // SHADINGCONFWIDGET_H
