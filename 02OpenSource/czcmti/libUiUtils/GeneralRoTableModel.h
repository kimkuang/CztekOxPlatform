#ifndef OSTESTRESULTTABLEMODEL_H
#define OSTESTRESULTTABLEMODEL_H
#include "library_global.h"
#include <QAbstractTableModel>

class LIBRARY_API GeneralRoTableModel : public QAbstractTableModel
{
public:
    explicit GeneralRoTableModel(QObject *parent = 0);

    void RefreshTable(const QList<QStringList> &dataTable);

public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

//    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    QList<QStringList> m_dataTable;
};

#endif // OSTESTRESULTTABLEMODEL_H
