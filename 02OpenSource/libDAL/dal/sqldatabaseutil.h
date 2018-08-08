#ifndef SQLDATABASEUTIL_H
#define SQLDATABASEUTIL_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QFile>
#include "library_global.h"

class LIBRARY_API SqlDatabaseUtil
{
public:
    static bool CreateDbConnections(const QString &connectionName, const QString &connectionString,
                                    bool isEncrypted = true);
    static QString GetDatabaseKey();
    static QSqlDatabase GetDatabase(const QString &connectionName);

public:
    static QString TangShi;

private:
    SqlDatabaseUtil();
};

#endif // SQLDATABASEUTIL_H
