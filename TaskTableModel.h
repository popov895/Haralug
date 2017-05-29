#ifndef TASKTABLEMODEL_H
#define TASKTABLEMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

// TaskTableModel
class TaskTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TaskTableModel(QObject *parent = Q_NULLPTR);

    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;

private:
    void emitDataChanged(const int row);

    static const QVariantList _headerData;

    int _tasksCount;
};

// TaskFilterProxyModel
class TaskFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TaskFilterProxyModel(QObject *parent = Q_NULLPTR);

    bool removeRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE;
};

#endif // TASKTABLEMODEL_H
