#ifndef VERSIONDAL_H
#define VERSIONDAL_H

#include "entities/version.h"
#include "sqldatabaseutil.h"
#include <QSqlQuery>

class LIBRARY_API VersionDAL
{
public:
    VersionDAL();
    ~VersionDAL();
    bool SelectById(Version &version);

private:
    QSqlDatabase m_systemDb;
    QSqlQuery *m_sqlQuery;
};

#endif // VERSIONDAL_H
