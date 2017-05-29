#include "Crypto.h"

#include <openssl/err.h>

using namespace Crypto;

// Exception

Exception::Exception(const Exception::Error error, const QString &errorMessage)
    : _error(error)
    , _errorMessage(errorMessage)
{}

// Base

void Base::throwLastError()
{
    throw Exception(Exception::Error::OpenSslError, ERR_error_string(ERR_peek_last_error(), Q_NULLPTR));
}

// Cipher

Cipher::Cipher(const EVP_CIPHER_CTX &context)
    : Base()
    , _context(context)
{}

QByteArray Cipher::update(const QByteArray &data)
{
    auto length = data.length();
    if (0 == length)
        return QByteArray();

    QByteArray buffer(length + EVP_MAX_BLOCK_LENGTH, 0);
    if (!EVP_CipherUpdate(&_context, (uchar*)buffer.data(), &length, (const uchar*)data.constData(), length))
        throwLastError();

    return buffer.left(length);
}

QByteArray Cipher::updateFinal()
{
    auto length = 0;
    QByteArray buffer(EVP_MAX_BLOCK_LENGTH, 0);
    if (!EVP_CipherFinal(&_context, (uchar*)buffer.data(), &length))
        throwLastError();

    return buffer.left(length);
}

// Digest

Digest::Digest(const EVP_MD_CTX &context)
    : Base()
    , _context(context)
{}

void Digest::update(const QByteArray &data)
{
    auto length = data.length();
    if ((length > 0) && !EVP_DigestUpdate(&_context, data.constData(), length))
        throwLastError();
}

QByteArray Digest::updateFinal()
{
    auto length = 0u;
    QByteArray buffer(EVP_MAX_MD_SIZE, 0);
    if (!EVP_DigestFinal(&_context, (uchar*)buffer.data(), &length))
        throwLastError();

    return buffer.left(length);
}

// Signer

Signer::Signer(const EVP_MD_CTX &context, const KeyPtr &key)
    : Base()
    , _context(context)
    , _key(key)
{}

void Signer::update(const QByteArray &data)
{
    auto length = data.length();
    if ((length > 0) && !EVP_DigestSignUpdate(&_context, data.constData(), length))
        throwLastError();
}

QByteArray Signer::updateFinal()
{
    size_t length = 0;
    QByteArray buffer(EVP_MAX_MD_SIZE, 0);
    if (!EVP_DigestSignFinal(&_context, (uchar*)buffer.data(), &length))
        throwLastError();

    return buffer.left(length);
}

// Factory

Factory::Factory()
    : Base()
{
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
}

Factory::~Factory()
{
    EVP_cleanup();
    ERR_free_strings();
}

Factory& Factory::instance()
{
    static Factory instance;

    return instance;
}

QByteArray Factory::hash(const QByteArray &data, const Factory::SHA sha)
{
    if (data.isEmpty())
        return QByteArray();

    DigestPtr digest(instance().createDigest(sha));
    Q_ASSERT(digest);
    digest->update(data);

    return digest->updateFinal();
}

QByteArray Factory::sign(const QByteArray &data, const QString &password)
{
    if (data.isEmpty())
        return QByteArray();

    SignerPtr signer(instance().createSigner(password));
    Q_ASSERT(signer);

    return signer->updateFinal();
}

QByteArray Factory::encrypt(const QByteArray &data, const QString &password)
{
    if (data.isEmpty())
        return QByteArray();

    CipherPtr cipher(instance().createCipher(password));
    Q_ASSERT(cipher);
    auto buffer = cipher->update(data);

    return (buffer + cipher->updateFinal());
}

QByteArray Factory::decrypt(const QByteArray &data, const QString &password)
{
    if (data.isEmpty())
        return QByteArray();

    CipherPtr cipher(instance().createCipher(password, false));
    Q_ASSERT(cipher);
    auto buffer = cipher->update(data);

    return (buffer + cipher->updateFinal());
}

CipherPtr Factory::createCipher(const QString &password, const bool encrypt)
{
    if (password.isEmpty())
        throw Exception(Exception::Error::EmptyPasswordError, "The password shouldn't be empty");

    const auto passwordHash = hash(password.toUtf8());
    Q_ASSERT(!passwordHash.isEmpty());

    EVP_CIPHER_CTX context;
    if (!EVP_CipherInit(&context, EVP_aes_256_cbc(), (const uchar*)passwordHash.constData(), Q_NULLPTR, encrypt))
        throwLastError();

    return CipherPtr(new Cipher(context));
}

DigestPtr Factory::createDigest(const Factory::SHA sha)
{
    static const EVP_MD* md[] = {
        EVP_sha256(),
        EVP_sha512()
    };

    EVP_MD_CTX context;
    if (!EVP_DigestInit(&context, md[static_cast<int>(sha)]))
        throwLastError();

    return DigestPtr(new Digest(context));
}

SignerPtr Factory::createSigner(const QString &password)
{
    if (password.isEmpty())
        throw Exception(Exception::Error::EmptyPasswordError, "The password shouldn't be empty");

    const auto passwordHash = hash(password.toUtf8());
    Q_ASSERT(!passwordHash.isEmpty());

    KeyPtr key(EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, Q_NULLPTR, (const uchar*)passwordHash.constData(), passwordHash.length()), EVP_PKEY_free);
    if (!key)
        throwLastError();

    EVP_MD_CTX context;
    if (!EVP_DigestInit(&context, EVP_sha512()))
        throwLastError();

    if (!EVP_DigestSignInit(&context, Q_NULLPTR, EVP_sha512(), Q_NULLPTR, key.data())) {
        EVP_MD_CTX_cleanup(&context);
        throwLastError();
    }

    return SignerPtr(new Signer(context, key));
}
