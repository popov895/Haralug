#include "Utils.h"

#include <cmath>

// Utils

int Utils::passwordStrength(const QString &password)
{
    if (password.isEmpty())
        return 0;

    static const QString lower   = "abcdefghijklmnopqrstuvwxyz";
    static const QString upper   = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const QString symbols = "~`!@#$%^&*()-_+=";
    static const QString digits  = "1234567890";

    enum Options {
        HasLower   = 1 << 0,
        HasUpper   = 1 << 1,
        HasDigits  = 1 << 2,
        HasSymbols = 1 << 3,
        HasOther   = 1 << 4
    };

    int options = 0;

    foreach (const QChar &i, password) {
        if (lower.contains(i))
            options |= HasLower;
        else if (upper.contains(i))
            options |= HasUpper;
        else if (digits.contains(i))
            options |= HasDigits;
        else if (symbols.contains(i))
            options |= HasSymbols;
        else
            options |= HasOther;
    }

    int strength = 0;

    if (options & HasLower)
        strength += lower.length();

    if (options & HasUpper)
        strength += upper.length();

    if (options & HasSymbols)
        strength += symbols.length();

    if (options & HasDigits)
        strength += digits.length();

    if (options & HasOther)
        strength += 0x7f - 0x20 - (lower.length() + upper.length() + symbols.length() + digits.length());

    strength = std::floor(std::log(strength) * (password.length() / std::log(2)));

    if (strength >= 128)
        return 4;

    if (strength >= 64)
        return 3;

    if (strength >= 56)
        return 2;

    return 1;
}
