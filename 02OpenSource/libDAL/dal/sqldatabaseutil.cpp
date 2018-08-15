#include "sqldatabaseutil.h"
#include <QDebug>
#include <QCoreApplication>

bool SqlDatabaseUtil::CreateDbConnections(const QString &connectionName, const QString &connectionString,
                                          bool isEncrypted)
{
    QMap<QString, QString> keyValMap;
    parseStringToKeyValueMap(keyValMap, connectionString, ";");

    QString databaseName, driverName;
    if (keyValMap.contains("DatabaseName"))
        databaseName = keyValMap["DatabaseName"];
    else {
        qCritical("No [DatabaseName] parameter in connection string.");
        return false;
    }
    if (keyValMap.contains("DriverName"))
        driverName = keyValMap["DriverName"];
    else {
        qCritical("No [DriverName] parameter in connection string.");
        return false;
    }
    QSqlDatabase database;
    if (QSqlDatabase::contains(connectionName))
        database = QSqlDatabase::database(connectionName, true);
    else
        database = QSqlDatabase::addDatabase(driverName, connectionName);
    if (driverName == "QMYSQL")
    {
        QString hostName, userName;
        if (keyValMap.contains("HostName"))
            hostName = keyValMap["HostName"];
        else {
            qCritical("No [HostName] parameter in connection string.");
            return false;
        }
        if (keyValMap.contains("UserName"))
            userName = keyValMap["UserName"];
        else {
            qCritical("No [UserName] parameter in connection string.");
            return false;
        }
        database.setHostName(hostName);
        database.setUserName(userName);
    }
    else if ((driverName == "QSQLITE") || (driverName == "SQLITECIPHER"))
    {
        databaseName = qApp->applicationDirPath() + "/" + databaseName;
        if (!QFile::exists(databaseName)) {
            qCritical()<<QString("Database file[%1] does not exist!").arg(databaseName);
            return false;
        }
    }
    else {
        qCritical()<<QString("Unkown database driver name[%1]!").arg(driverName);
        return false;
    }
    database.setDatabaseName(databaseName);
    if (isEncrypted) {
        QString password = "dummy"; // only for test
        database.setPassword(password);
    }
    database.open();
    if (!database.isOpen()) {
        qCritical("Database[%s] has not opened.", databaseName.toStdString().c_str());
        return false;
    }
    qInfo("Database[%s] connection has initialized.", databaseName.toStdString().c_str());
    return true;
}

QSqlDatabase SqlDatabaseUtil::GetDatabase(const QString &connectionName)
{
    return QSqlDatabase::database(connectionName);
}

void SqlDatabaseUtil::parseStringToKeyValueMap(QMap<QString, QString> &keyValMap, const QString &str, const QString &sep)
{
    QStringList slTemp = str.split(sep, QString::SkipEmptyParts, Qt::CaseSensitive);
    foreach (QString s, slTemp) {
        int index = s.indexOf(QChar('='));
        QString key = s.mid(0, index).trimmed();
        QString val = s.mid(index + 1).trimmed();
        if (!key.isEmpty())
            keyValMap.insert(key, val);
    }
}

