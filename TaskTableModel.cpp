#include "TaskTableModel.h"

#include <QPersistentModelIndex>
#include <QPointer>

#include "TaskManager.h"

// TaskTableModel

const QVariantList TaskTableModel::_headerData = {
    "Source",
    "Destination",
    "Status"
};

TaskTableModel::TaskTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , _tasksCount(0)
{
    qRegisterMetaType<QVector<int>>();

    connect(TaskManager::instance(), &TaskManager::taskAdded, [this] () {
        QPointer<Task> task(TaskManager::instance()->tasks().value(_tasksCount));
        Q_ASSERT(task);

        const auto row = _tasksCount;
        beginInsertRows(QModelIndex(), row, row);
        ++_tasksCount;
        endInsertRows();

        QPersistentModelIndex taskIndex(index(row, 0));

        connect(task, &Task::outputFileChanged, [this, taskIndex] () {
            Q_ASSERT(taskIndex.isValid());
            emitDataChanged(taskIndex.row());
        });
        connect(task, &Task::progressChanged, [this, taskIndex] () {
            Q_ASSERT(taskIndex.isValid());
            emitDataChanged(taskIndex.row());
        });
        connect(task, &Task::stateChanged, [this, taskIndex] () {
            Q_ASSERT(taskIndex.isValid());
            emitDataChanged(taskIndex.row());
        });
    });
}

int TaskTableModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid() ? 0 : _headerData.size());
}

QVariant TaskTableModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const int row = index.row();
        Q_ASSERT((row >= 0) && (row < _tasksCount));

        const QPointer<Task> task(TaskManager::instance()->tasks().value(index.row()));
        Q_ASSERT(task);

        switch (role) {
            case Qt::ToolTipRole: // fall through
            case Qt::DisplayRole:
            {
                switch (index.column()) {
                    case 0:
                        return task->inputFile();
                    case 1:
                        return ((Task::State::Succeded == task->state()) ? task->outputFile() : QString());
                    case 2:
                    {
                        switch (task->state()) {
                            case Task::State::New:
                                return "New";
                            case Task::State::Queued:
                                return "Queued";
                            case Task::State::Running:
                                return ((Qt::DisplayRole == role) ? QString() : QString("%1%").arg(task->progress()));
                            case Task::State::Succeded:
                                return "Succeded";
                            case Task::State::Failed:
                                return QString("Failed (%1)").arg(task->lastError());
                            default:
                                break;
                        }
                    }
                    default:
                        break;
                }
            }
            default:
                break;
        }
    }

    return QVariant();
}

Qt::ItemFlags TaskTableModel::flags(const QModelIndex &index) const
{
    return (index.isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable) : Qt::NoItemFlags);
}

QVariant TaskTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((Qt::Horizontal == orientation) && (Qt::DisplayRole == role))
        return _headerData.value(section);

    return QAbstractTableModel::headerData(section, orientation, role);
}

bool TaskTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;

    Q_ASSERT((row >= 0) && (row < _tasksCount));
    while (count-- > 0) {
        beginRemoveRows(QModelIndex(), row, row);
        TaskManager::instance()->removeTask(row);
        --_tasksCount;
        endRemoveRows();
    }

    return true;
}

int TaskTableModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() ? 0 : _tasksCount);
}

void TaskTableModel::emitDataChanged(const int row)
{
    Q_ASSERT((row >= 0) && (row < _tasksCount));

    Q_EMIT dataChanged(index(row, 0), index(row, _headerData.size() - 1), QVector<int>() << Qt::DisplayRole << Qt::ToolTipRole);
}

// TaskFilterProxyModel

TaskFilterProxyModel::TaskFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{}

bool TaskFilterProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
    auto model = sourceModel();
    if (Q_NULLPTR == model)
        return false;

    QModelIndexList indexes;
    const auto sourceParent = mapToSource(parent);
    for (auto i = 0; i < count; ++i)
        indexes << mapToSource(index(row + i, 0, sourceParent));

    for (auto i = indexes.rbegin(); i != indexes.rend(); ++i)
        model->removeRow(i->row());

    return true;
}

bool TaskFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto model = sourceModel();
    if (Q_NULLPTR != model) {
        const auto regExp = filterRegExp();
        for (auto i = 0; i < sourceModel()->columnCount(); ++i) {
            if (model->data(sourceModel()->index(sourceRow, i, sourceParent)).toString().contains(regExp))
                return true;
        }
    }

    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
