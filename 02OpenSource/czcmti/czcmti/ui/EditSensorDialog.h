#ifndef DIALOGEDITSENSOR_H
#define DIALOGEDITSENSOR_H

#include <QDialog>
#include "bll/sensorbll.h"

namespace Ui {
class DialogEditSensor;
}

class DialogEditSensor : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditSensor(QWidget *parent = 0);
    ~DialogEditSensor();

    inline void SetId(int id) {
        m_id = id;
        bindData(m_id);
    }

    inline void GetSchemeName(QString &schemeName)
    {
        schemeName = m_schemeName;
    }

private slots:
    void btnOK_clicked();
    void btnCancel_clicked();

private:
    Ui::DialogEditSensor *ui;
    int m_id;
    SensorBLL *m_sensorBll;
    QString m_schemeName;

    void initComboBoxData();
    void initBoolFrame();
    void bindData(int id);
};

#endif // DIALOGEDITSENSOR_H
