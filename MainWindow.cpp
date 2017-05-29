#include "MainWindow.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QListView>
#include <QMessageBox>
#include <QMimeData>
#include <QPointer>

#include <functional>

#include <AboutDialog.h>
#include "PasswordDialog.h"
#include "Settings.h"
#include "TaskManager.h"
#include "TaskProgressItemDelegate.h"
#include "TaskTableModel.h"
#include "ThreadPool.h"
#include "Utils.h"

// MainWindow

const QString MainWindow::_keyHeaderState = "headerState";

MainWindow::MainWindow()
    : QMainWindow()
    , _model(new TaskTableModel(this))
    , _filterModel(new TaskFilterProxyModel(this))
{
    setupUi(this);
    setWindowTitle(QApplication::applicationName());

    _filterModel->setSourceModel(_model);

    toolBar->addAction(actionAdd);
    toolBar->addAction(actionRemove);
    toolBar->addSeparator();
    toolBar->addAction(actionStart);
    toolBar->addAction(actionStop);
    toolBar->addSeparator();
    toolBar->addAction(actionPassword);
    toolBar->addSeparator();
    toolBar->addAction(actionAbout);
    toolBar->addWidget(widgetSearch);

    treeViewTasks->installEventFilter(this);
    treeViewTasks->setItemDelegate(new TaskProgressItemDelegate(_model->columnCount() - 1, this));
    treeViewTasks->setModel(_filterModel);
    connect(treeViewTasks->selectionModel(), &QItemSelectionModel::selectionChanged, [this] () {
        actionRemove->setEnabled(treeViewTasks->selectionModel()->hasSelection());
    });

    treeViewTasks->header()->restoreState(Settings::instance().value(_keyHeaderState).toByteArray());
    connect(treeViewTasks->header(), &QHeaderView::geometriesChanged, [this] () {
        Settings::instance().setValue(_keyHeaderState, treeViewTasks->header()->saveState());
    });

    auto updateControls = [this] () {
        auto state = ThreadPool::instance()->state();
        actionStart->setVisible(ThreadPool::State::Stopped == state);
        actionStart->setEnabled(!TaskManager::instance()->tasks().isEmpty());
        actionStop->setVisible(ThreadPool::State::Running == state);
        actionPassword->setEnabled(ThreadPool::State::Stopped == state);
    };

    connect(TaskManager::instance(), &TaskManager::taskAdded, updateControls);
    connect(TaskManager::instance(), &TaskManager::taskRemoved, updateControls);

    connect(ThreadPool::instance(), &ThreadPool::stateChanged, updateControls);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    auto threadPool = ThreadPool::instance();
    if (ThreadPool::State::Running == threadPool->state()) {
        QMessageBox messageBox(this);
        messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        messageBox.setText("All running tasks will not be complete. Do you really want to exit the application?");
        if (QMessageBox::Yes == messageBox.exec())
            threadPool->stop();
        else
            event->ignore();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    foreach (const auto url, event->mimeData()->urls())
        addPath(url.toLocalFile());

    activateWindow();
    raise();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == treeViewTasks) && (QEvent::KeyPress == event->type()) && (Qt::Key_Delete == static_cast<QKeyEvent*>(event)->key()) && treeViewTasks->selectionModel()->hasSelection())
        actionRemove->trigger();

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::addPath(const QString &path)
{
    Q_ASSERT(!path.isEmpty());

    std::function<void(QString)> scanFolder;
    scanFolder = [this, scanFolder] (const QString &folder) {
        foreach (const auto fileInfo, QDir(folder).entryInfoList()) {
            if (fileInfo.isDir()) {
                const auto fileName = fileInfo.fileName();
                if (("." != fileName) && (".." != fileName))
                    scanFolder(fileInfo.absoluteFilePath());
            } else {
                TaskManager::instance()->addTask(fileInfo.absoluteFilePath());
            }
        }
    };

    const QFileInfo fileInfo(path);
    if (fileInfo.isDir())
        scanFolder(fileInfo.absoluteFilePath());
    else if (!fileInfo.isSymLink())
        TaskManager::instance()->addTask(fileInfo.absoluteFilePath());
}

void MainWindow::on_filterEdit_textChanged(const QString &text)
{
    _filterModel->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
}

void MainWindow::on_actionAdd_triggered()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if (dialog.exec()) {
        for (const auto fileName : dialog.selectedFiles())
            addPath(fileName);
    }
}

void MainWindow::on_actionRemove_triggered()
{
    const auto rows = treeViewTasks->selectionModel()->selectedRows();
    Q_ASSERT(!rows.isEmpty());
    for (auto i = rows.rbegin(); i != rows.rend(); ++i)
        _filterModel->removeRow(i->row());
}

void MainWindow::on_actionStart_triggered()
{
    if (Settings::instance().password().isEmpty() && !PasswordDialog(this).exec())
        return;

    actionStart->setVisible(false);

    ThreadPool::instance()->start();
}

void MainWindow::on_actionStop_triggered()
{
    ThreadPool::instance()->stop();
}

void MainWindow::on_actionPassword_triggered()
{
    PasswordDialog(this).exec();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::on_treeViewTasks_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const QPointer<Task> task(TaskManager::instance()->tasks().value(index.row()));
    Q_ASSERT(task);
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(task->inputFile()).absolutePath()));
}
