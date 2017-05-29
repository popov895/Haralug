#include "PasswordGenerator.h"

#include <algorithm>
#include <random>

// PasswordGenerator

QString PasswordGenerator::generate(Options option, int length)
{
    Q_ASSERT(length > 5);

    QString symbols = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    if (option.testFlag(DigitsOption))
        symbols += "0123456789";

    if (option.testFlag(MinusOption))
        symbols += "-";

    if (option.testFlag(UnderlineOption))
        symbols += "_";

    if (option.testFlag(SpaceOption))
        symbols += " ";

    if (option.testFlag(SpecialOption))
        symbols += "!@#$%^&*+=;:'\",./?\\|`~";

    if (option.testFlag(BracketsOption))
        symbols += "[]{}()<>";

    while (symbols.length() < length)
        symbols += symbols;

    static std::mt19937 engine((std::random_device())());
    std::shuffle(symbols.begin(), symbols.end(), engine);

    return symbols.left(length);
}
