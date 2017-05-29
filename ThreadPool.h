#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QList>
#include <QMutex>
#include <QObject>
#include <QRunnable>
#include <QWaitCondition>

#include "TaskManager.h"

class QThreadPool;

// TaskJob
class TaskJob : public QObject, public QRunnable
{
    Q_OBJECT

    friend class ThreadPool;

private:
    TaskJob(TaskPtr task, QObject *parent);

public:
    bool isRunning() const { return _running; }
    void requestInterruption() { _interruptionRequested = true; }

    Q_SIGNAL void finished();

protected:
    void run() Q_DECL_OVERRIDE;

private:
    static void improveFilePath(QString &filePath, bool encrypted);

    void doJob() noexcept;

    void setTaskOutputFile(const QString &outputFile);
    void setTaskLastError(const QString &lastError);
    void setTaskProgress(int progress);
    void setTaskState(Task::State state);

    TaskPtr _task;
    std::atomic_bool _running;
    std::atomic_bool _interruptionRequested;
    QWaitCondition _finished;
};

using TaskJobPtr = TaskJob*;

// ThreadPool
class ThreadPool : public QObject
{
    Q_OBJECT

private:
    ThreadPool();
    virtual ~ThreadPool();

public:
    enum class State {
        Starting,
        Running,
        Stopping,
        Stopped
    };

    static ThreadPool* instance();

    ThreadPool::State state() const { return _state; }
    Q_SIGNAL void stateChanged(ThreadPool::State state);

    bool addTask(TaskPtr task);
    void removeTask(int index);

    bool start();
    bool stop();

private:
    struct JobInfo {
        TaskPtr task;
        TaskJobPtr job;
    };

    ThreadPool::State _state;
    QList<JobInfo> _jobs;
    QThreadPool *_threadPool;
    QMutex _jobFinishedMutex;
};

Q_DECLARE_METATYPE(ThreadPool::State)

#endif // THREADPOOL_H
