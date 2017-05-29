#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QList>
#include <QObject>

// Task
class Task : public QObject
{
    Q_OBJECT

    friend class TaskManager;

private:
    Task(const QString &inputFile, QObject *parent = Q_NULLPTR);
    ~Task() {}

public:
    enum class State {
        New,
        Queued,
        Running,
        Succeded,
        Failed
    };

    const QString& inputFile() const { return _inputFile; }

    QString outputFile() const { return _outputFile; }
    Q_SLOT void setOutputFile(const QString &outputFile);
    Q_SIGNAL void outputFileChanged(const QString &outputFile);

    QString lastError() const { return _lastError; }
    Q_SLOT void setLastError(const QString &lastError);
    Q_SIGNAL void lastErrorChanged(const QString &lastError);

    int progress() const { return _progress; }
    Q_SLOT void setProgress(const int progress);
    Q_SIGNAL void progressChanged(const int progress);

    Task::State state() const { return _state; }
    Q_SLOT void setState(const Task::State state);
    Q_SIGNAL void stateChanged(const Task::State state);

    void setParent(QObject *parent) Q_DECL_EQ_DELETE;

private:
    const QString _inputFile;
    QString _outputFile;
    QString _lastError;
    int _progress;
    Task::State _state;
};

Q_DECLARE_METATYPE(Task::State)

using TaskPtr = Task*;
using TaskList = QList<TaskPtr>;

// TaskManager
class TaskManager : public QObject
{
    Q_OBJECT

private:
    TaskManager();
    virtual ~TaskManager() {}

public:
    static TaskManager* instance();

    TaskList tasks() const { return _tasks; }

    void addTask(const QString &inputFile);
    Q_SIGNAL void taskAdded();

    void removeTask(int index);
    Q_SIGNAL void taskRemoved(int index);

private:
    TaskList _tasks;
};

#endif // TASKMANAGER_H
