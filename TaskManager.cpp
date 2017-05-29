#include "TaskManager.h"

#include <QPointer>

#include "ThreadPool.h"

// Task

Task::Task(const QString &inputFile, QObject *parent)
    : QObject(parent)
    , _inputFile(inputFile)
    , _progress(0)
    , _state(State::New)
{}

void Task::setOutputFile(const QString &outputFile)
{
    if (outputFile != _outputFile)
        Q_EMIT outputFileChanged(_outputFile = outputFile);
}

void Task::setLastError(const QString &lastError)
{
    if (lastError != _lastError)
        Q_EMIT lastErrorChanged(_lastError = lastError);
}

void Task::setProgress(const int progress)
{
    if (progress != _progress)
        Q_EMIT progressChanged(_progress = progress);
}

void Task::setState(const Task::State state)
{
    if (state != _state)
        Q_EMIT stateChanged(_state = state);
}

// TaskManager

TaskManager::TaskManager()
    : QObject()
{
    qRegisterMetaType<Task::State>();
}

TaskManager* TaskManager::instance()
{
    static TaskManager instance;

    return (&instance);
}

void TaskManager::addTask(const QString &inputFile)
{
    auto task = new Task(inputFile, this);
    _tasks << task;
    Q_EMIT taskAdded();
    ThreadPool::instance()->addTask(task);
}

void TaskManager::removeTask(int index)
{
    QPointer<Task> task(_tasks.takeAt(index));
    Q_ASSERT(task);
    task->blockSignals(true);
    ThreadPool::instance()->removeTask(index);
    Q_EMIT taskRemoved(index);
    task->deleteLater();
}
