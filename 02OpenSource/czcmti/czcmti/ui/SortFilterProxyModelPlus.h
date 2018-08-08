#ifndef SORTFILTERPROXYMODELPLUS
#define SORTFILTERPROXYMODELPLUS
#include <QSortFilterProxyModel>

class SortFilterProxyModelPlus : public QSortFilterProxyModel
{
public:
    explicit SortFilterProxyModelPlus(QObject *parent = 0) : QSortFilterProxyModel(parent)
    {
    }

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
    {
        bool filter = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

        if (filter) {
            return true;
        }
        else {
            // 如果子项中有一项通过，则本身也通过
            QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);
            for (int k=0; k<sourceModel()->rowCount(source_index); k++) // 遍历其子项
            {
                if (QSortFilterProxyModel::filterAcceptsRow(k, source_index)) {
                    return true;
                }
            }
            // 如果父项通过，则本身也通过
            if (source_parent.isValid()) {
                if (QSortFilterProxyModel::filterAcceptsRow(source_parent.row(), source_parent.parent())) {
                    return true;
                }
            }
        }

        return false;
    }

    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        // NOTICE: 在sensor配置页面中，treeview默认设置时是倒序的，未查明原因，此处反逻辑处理
        return sourceModel()->data(left).toString() > sourceModel()->data(right).toString();
    }
};

#endif // SORTFILTERPROXYMODELPLUS

