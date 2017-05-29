#include "TaskProgressItemDelegate.h"

#include <QApplication>
#include <QPointer>
#include <QStyleOptionProgressBar>

#include "TaskManager.h"

// TaskProgressItemDelegate

TaskProgressItemDelegate::TaskProgressItemDelegate(int column, QObject *parent)
    : QStyledItemDelegate(parent)
    , _column(column)
{}

void TaskProgressItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto itemOption = option;
    itemOption.state &= ~QStyle::State_HasFocus;
    QStyledItemDelegate::paint(painter, itemOption, index);

    if (index.isValid() && (_column == index.column())) {
        QPointer<Task> task(TaskManager::instance()->tasks().value(index.row()));
        if (task && (Task::State::Running == task->state())) {
            auto widget = qobject_cast<QWidget*>(itemOption.styleObject);
            Q_CHECK_PTR(widget);

            static QStyleOptionProgressBar progressBarOption;
            progressBarOption.initFrom(widget);
            progressBarOption.rect = itemOption.rect.adjusted(2, 2, -2, -2);
            progressBarOption.state = itemOption.state;
            progressBarOption.minimum = 0;
            progressBarOption.maximum = 100;
            progressBarOption.progress = task->progress();

            QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
        }
    }
}
