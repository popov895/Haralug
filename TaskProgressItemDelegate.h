#ifndef TASKPROGRESSITEMDELEGATE_H
#define TASKPROGRESSITEMDELEGATE_H

#include <QStyledItemDelegate>

// TaskProgressItemDelegate
class TaskProgressItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TaskProgressItemDelegate(int column, QObject *parent = Q_NULLPTR);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    const int _column;
};

#endif // TASKPROGRESSITEMDELEGATE_H
