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
    static QSqlDatabase GetDatabase(const QString &connectionName);

private:
    SqlDatabaseUtil();
    static void parseStringToKeyValueMap(QMap<QString, QString> &keyValMap, const QString &str, const QString &sep);
};

#endif // SQLDATABASEUTIL_H
