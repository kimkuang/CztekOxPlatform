#ifndef SENSORDAL_H
#define SENSORDAL_H

#include "entities/sensor.h"
#include "sqldatabaseutil.h"
#include <QSqlQuery>
#include <QList>
#include <QTime>

class SensorDAL
{
public:
    SensorDAL();
    ~SensorDAL();

    bool Insert(const Sensor &sensor);
    bool Delete(int id);
    bool Update(const Sensor &sensor, const QString &whereClause, bool updateDef = false);
    bool Select(Sensor &sensor, const QString &whereClause);
    bool SelectList(QList<Sensor> &sensors, const QString &whereClause = "", const QString &orderByClause = "");
    bool SelectList_IdVendorSchemeName(QList<Sensor> &sensors, const QString &whereClause = "");
    int SelectCount(const QString &whereClause);

private:
    QSqlDatabase m_systemDb;
    QSqlQuery *m_sqlQuery;
    QTime m_time;
};

#endif // SENSORDAL_H
