#ifndef DATABASESETTINGWIDGET_H
#define DATABASESETTINGWIDGET_H

#include <QWidget>

namespace Ui {
class DatabaseSettingWidget;
}

class DatabaseSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DatabaseSettingWidget(QWidget *parent = 0);
    ~DatabaseSettingWidget();

private:
    Ui::DatabaseSettingWidget *ui;
};

#endif // DATABASESETTINGWIDGET_H
