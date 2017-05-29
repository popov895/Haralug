#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_MainWindow.h"

class TaskTableModel;
class TaskFilterProxyModel;

// MainWindow
class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    void addPath(const QString &path);

    static const QString _keyHeaderState;

    TaskTableModel *_model;
    TaskFilterProxyModel *_filterModel;

    // GUI
    Q_SLOT void on_filterEdit_textChanged(const QString &text);
    Q_SLOT void on_actionAdd_triggered();
    Q_SLOT void on_actionRemove_triggered();
    Q_SLOT void on_actionStart_triggered();
    Q_SLOT void on_actionStop_triggered();
    Q_SLOT void on_actionPassword_triggered();
    Q_SLOT void on_actionAbout_triggered();
    Q_SLOT void on_treeViewTasks_doubleClicked(const QModelIndex &index);
};

#endif // MAINWINDOW_H
