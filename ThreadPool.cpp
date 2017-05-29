#include "ThreadPool.h"

#include <QFileInfo>
#include <QPointer>
#include <QThreadPool>

#include "Crypto.h"
#include "Settings.h"
#include "Utils.h"

using namespace Crypto;

// TaskJob

TaskJob::TaskJob(TaskPtr task, QObject *parent)
    : QObject(parent)
    , QRunnable()
    , _task(task)
    , _running(false)
{
    setAutoDelete(false);
}

void TaskJob::run()
{
    _interruptionRequested = false;
    _running = true;

    doJob();

    _running = false;
    _finished.wakeAll();

    Q_EMIT finished();
}

void TaskJob::doJob() noexcept
{
    Q_ASSERT(Q_NULLPTR != _task);
    setTaskState(Task::State::Running);

    auto encrypt = true;
    auto outputFileName = _task->inputFile();

    static const QString encryptedFileExt = ".haralug";

    if (outputFileName.endsWith(encryptedFileExt)) {
        encrypt = false;
        outputFileName = outputFileName.left(outputFileName.length() - encryptedFileExt.length());
    } else {
        outputFileName += encryptedFileExt;
    }

    improveFilePath(outputFileName, encrypt);

    QFile inputFile(_task->inputFile());
    if (!inputFile.open(QFile::ReadOnly)) {
        setTaskLastError(QString("'%1': %2").arg(inputFile.fileName()).arg(inputFile.errorString()));
        setTaskState(Task::State::Failed);

        return;
    }

    const auto signature = Settings::instance().signature();
    Q_ASSERT(!signature.isEmpty());

    if (!encrypt && (inputFile.read(signature.size()) != signature)) {
            setTaskLastError("Wrong password");
            setTaskState(Task::State::Failed);

            return;
    }

    QFile outputFile(outputFileName);
    if (!outputFile.open(QFile::WriteOnly)) {
        setTaskLastError(QString("'%1': %2").arg(outputFileName).arg(outputFile.errorString()));
        setTaskState(Task::State::Failed);

        return;
    }

    if (encrypt)
        outputFile.write(signature);

    const auto password = Settings::instance().password();

    try {
        CipherPtr cipher(Factory::instance().createCipher(password, encrypt));
        Q_ASSERT(cipher);

        auto progress = 0;
        while (!inputFile.atEnd()) {
            if (_interruptionRequested) {
                setTaskLastError("Aborted");
                setTaskState(Task::State::Failed);
                outputFile.close();
                outputFile.remove();

                return;
            }

            static const auto bufferSize = 1024;
            outputFile.write(cipher->update(inputFile.read(bufferSize)));
            const auto newProgress = 100 * inputFile.pos() / inputFile.size();
            if (newProgress > progress)
                setTaskProgress(progress = newProgress);
        }

        outputFile.write(cipher->updateFinal());
        setTaskOutputFile(outputFileName);
        setTaskState(Task::State::Succeded);
    } catch (const Exception &e) {
        setTaskLastError(e.errorMessage());
        setTaskState(Task::State::Failed);
    }
}

void TaskJob::improveFilePath(QString &filePath, bool encrypted)
{
    QFileInfo info(filePath);

    const auto absolutePath = info.absolutePath();
    const auto fileName     = info.fileName();

    QString baseName, suffix;
    QRegExp regExp;

    if (encrypted) {
        regExp.setPattern("^(.+)\\.([^\\.]+\\.[^\\.]+)$");
        if (regExp.exactMatch(fileName)) {
            baseName = regExp.cap(1);
            suffix   = regExp.cap(2);
        } else {
            regExp.setPattern("^(\\.[^\\.]+)\\.([^\\.]+)$");
            if (regExp.exactMatch(fileName)) {
                baseName = regExp.cap(1);
                suffix   = regExp.cap(2);
            } else {
                baseName = info.baseName();
                suffix   = info.suffix();
            }
        }
    } else {
        regExp.setPattern("^(.+)\\.([^\\.]+)$");
        if (regExp.exactMatch(fileName)) {
            baseName = regExp.cap(1);
            suffix   = regExp.cap(2);
        } else {
            regExp.setPattern("^(\\.[^\\.]+)$");
            if (regExp.exactMatch(fileName))
                baseName = regExp.cap(1);
            else
                baseName = fileName;
        }
    }

    quint64 index = 1;
    while (QFile::exists(filePath))
        filePath = QString("%1/%2-%3%4").arg(absolutePath).arg(baseName).arg(index++).arg(suffix.isEmpty() ? QString() : QString(".%1").arg(suffix));
}

void TaskJob::setTaskOutputFile(const QString &outputFile)
{
    Q_ASSERT(Q_NULLPTR != _task);
    QMetaObject::invokeMethod(_task, "setOutputFile", Q_ARG(QString, outputFile));
}

void TaskJob::setTaskLastError(const QString &lastError)
{
    Q_ASSERT(Q_NULLPTR != _task);
    QMetaObject::invokeMethod(_task, "setLastError", Q_ARG(QString, lastError));
}

void TaskJob::setTaskProgress(int progress)
{
    Q_ASSERT(Q_NULLPTR != _task);
    QMetaObject::invokeMethod(_task, "setProgress", Q_ARG(int, progress));
}

void TaskJob::setTaskState(Task::State state)
{
    Q_ASSERT(Q_NULLPTR != _task);
    QMetaObject::invokeMethod(_task, "setState", Q_ARG(Task::State, state));
}

// ThreadPool

ThreadPool::ThreadPool()
    : QObject()
    , _state(State::Stopped)
    , _threadPool(new QThreadPool(this))
{
    qRegisterMetaType<ThreadPool::State>();
}

ThreadPool::~ThreadPool()
{
    Q_ASSERT(ThreadPool::State::Stopped == _state);
}

ThreadPool* ThreadPool::instance()
{
    static ThreadPool instance;

    return (&instance);
}

bool ThreadPool::addTask(TaskPtr task)
{
    Q_ASSERT(Q_NULLPTR != task);

    for (auto i : _jobs) {
        if (i.task == task)
            return false;
    }

    auto job = new TaskJob(task, this);
    connect(job, &TaskJob::finished, this, [this, job] () {
        for (auto i : _jobs) {
            if (i.job->isRunning())
                return;
        }
        Q_EMIT stateChanged(_state = ThreadPool::State::Stopped);
    });
    _jobs << JobInfo { task, job };
    if (State::Running == _state)
        _threadPool->start(job);

    return true;
}

void ThreadPool::removeTask(int index)
{
    Q_ASSERT((index >= 0) && (index < _jobs.size()));

    QPointer<TaskJob> job(_jobs.takeAt(index).job);
    Q_ASSERT(job);
    _threadPool->cancel(job);
    if (job->isRunning()) {
        connect(job, &TaskJob::finished, job, &TaskJob::deleteLater);
        job->requestInterruption();
        QMutexLocker locker(&_jobFinishedMutex);
        job->_finished.wait(locker.mutex());
    } else {
        job->deleteLater();
    }
}

bool ThreadPool::start()
{
    if ((State::Stopped == _state) && !_jobs.isEmpty()) {
        Q_EMIT stateChanged(_state = State::Starting);

        for (auto &i : _jobs) {
            i.task->setState(Task::State::Queued);
            Q_ASSERT(!i.job->isRunning());
            _threadPool->start(i.job);
        }

        Q_EMIT stateChanged(_state = State::Running);

        return true;
    }

    return false;
}

bool ThreadPool::stop()
{
    if (State::Running == _state)
    {
        Q_EMIT stateChanged(_state = State::Stopping);

        _threadPool->clear();
        for (auto &i : _jobs) {
            if (i.job->isRunning()) {
                i.job->requestInterruption();
            } else {
                i.task->setLastError("Aborted");
                i.task->setState(Task::State::Failed);
            }
        }
        _threadPool->waitForDone();

        Q_EMIT stateChanged(_state = State::Stopped);

        return true;
    }

    return false;
}
