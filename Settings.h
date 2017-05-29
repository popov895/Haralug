#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariant>

class QSettings;

// Settings
class Settings : public QObject
{
    Q_OBJECT

private:
    Settings();
    virtual ~Settings() {}

public:
    static Settings& instance();

    const QString& password() const { return _password; }
    bool setPassword(const QString &password);

    const QByteArray& signature() const { return _signature; }

    QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
    void setValue(const QString &key, const QVariant &value);

private:
    QString _password;
    QByteArray _signature;
    QSettings *_settings;
};

#endif // SETTINGS_H
