#ifndef PASSWORDGENERATOR_H
#define PASSWORDGENERATOR_H

#include <QString>

// PasswordGenerator
class PasswordGenerator
{
    Q_DISABLE_COPY(PasswordGenerator)

private:
    PasswordGenerator() {}
    virtual ~PasswordGenerator() {}

public:
    enum Option
    {
        DigitsOption    = 1 << 0,
        MinusOption     = 1 << 1,
        UnderlineOption = 1 << 2,
        SpaceOption     = 1 << 3,
        SpecialOption   = 1 << 4,
        BracketsOption  = 1 << 5
    };
    Q_DECLARE_FLAGS(Options, Option)

    static QString generate(Options options, int length);
};

#endif // PASSWORDGENERATOR_H
