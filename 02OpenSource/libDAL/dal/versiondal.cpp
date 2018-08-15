#include "versiondal.h"
#include <QDebug>

VersionDAL::VersionDAL()
{
    m_systemDb = SqlDatabaseUtil::GetDatabase("system");
    if (!m_systemDb.isOpen())
        qCritical()<<"Database system has not opened.";
    m_sqlQuery = new QSqlQuery(m_systemDb);
}

VersionDAL::~VersionDAL()
{
    delete m_sqlQuery;
}

bool VersionDAL::SelectById(Version &version)
{
    QString sql = QString("select id, object_name, bin_version, bin_date, db_version, db_date from t_version where id=%1").arg(version.Id);
//    qDebug()<<sql;
    if (!m_sqlQuery->exec(sql))
        return false;
    if (m_sqlQuery->next()) {
//        version.Id = m_sqlQuery->value(0).toInt();
        version.ObjectName = m_sqlQuery->value(1).toString();
        version.BinVersion = m_sqlQuery->value(2).toString();
        version.BinDate = m_sqlQuery->value(3).toString();
        version.DbVersion = m_sqlQuery->value(4).toString();
        version.DbDate = m_sqlQuery->value(5).toString();
        return true;
    }
    return false;
}

