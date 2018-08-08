#include "GeneralRoTableModel.h"
#include <QDebug>

GeneralRoTableModel::GeneralRoTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

void GeneralRoTableModel::RefreshTable(const QList<QStringList> &dataTable)
{
    beginResetModel();
    m_dataTable = dataTable;
    endResetModel();
}

int GeneralRoTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_dataTable.size() - 1; // the first line is table header
}

int GeneralRoTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_dataTable.size() > 0 ? m_dataTable[0].size() : 0;
}

QVariant GeneralRoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        QStringList headerNames;
        if (m_dataTable.size() > 0)
            headerNames = m_dataTable[0];
        return section < headerNames.size() ? headerNames[section] : "";
    }
    else
        return QString::number(section + 1);
}

QVariant GeneralRoTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (m_dataTable.size() == 0))
        return QVariant();
    int rowIdx = index.row() + 1; // the first line is table header
    int colIdx = index.column();

    if (role == Qt::DisplayRole)
    {
        if ((m_dataTable.size() > rowIdx) && (m_dataTable[rowIdx].size() > colIdx))
            return QVariant(m_dataTable[rowIdx][colIdx]);
        return QVariant();
    }
    else if (role == Qt::TextAlignmentRole) {
        if (colIdx == 0) // item caption
            return (int)Qt::AlignCenter;
        else
            return (int)(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return QVariant();
}
