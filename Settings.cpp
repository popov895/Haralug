#include "Settings.h"

#include <QApplication>
#include <QSettings>

#include "Crypto.h"
#include "ThreadPool.h"

using namespace Crypto;

// Settings

Settings::Settings()
    : QObject()
    , _settings(new QSettings(this))
{}

Settings& Settings::instance()
{
    static Settings instance;

    return instance;
}

bool Settings::setPassword(const QString &password)
{
    Q_ASSERT(ThreadPool::State::Stopped == ThreadPool::instance()->state());

    if (password != _password) {
        try {
            _signature = Factory::sign(QApplication::applicationName().toUtf8(), password);
            _password = password;
        } catch (...) {
            return false;
        }
    }

    return true;
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue)
{
    return _settings->value(key, defaultValue);
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    _settings->setValue(key, value);
}
