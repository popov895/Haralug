#ifndef UTILS_H
#define UTILS_H

#include <QString>

// Utils
class Utils
{
    Q_DISABLE_COPY(Utils)

private:
    Utils() {}
    virtual ~Utils() {}

public:
    static int passwordStrength(const QString &password);
};

#endif // UTILS_H
