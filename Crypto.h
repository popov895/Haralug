#ifndef CRYPTO_H
#define CRYPTO_H

#include <QByteArray>
#include <QSharedPointer>
#include <QString>

#include <openssl/evp.h>

namespace Crypto
{
    // KeyPtr
    using KeyPtr = QSharedPointer<EVP_PKEY>;

    // Exception
    class Exception
    {
    public:
        enum class Error {
            EmptyPasswordError,
            OpenSslError
        };

    public:
        explicit Exception(const Exception::Error error, const QString &errorMessage);

        Exception::Error error() const { return _error; }
        const QString& errorMessage() const { return _errorMessage; }

    private:
        Exception::Error _error;
        QString _errorMessage;
    };

    // Base
    class Base
    {
    protected:
        virtual void throwLastError();
    };

    // Cipher
    class Cipher : public Base
    {
        Q_DISABLE_COPY(Cipher)

        friend class Factory;

    private:
        Cipher(const EVP_CIPHER_CTX &context);

    public:
        virtual ~Cipher() { EVP_CIPHER_CTX_cleanup(&_context); }

        QByteArray update(const QByteArray &data);
        QByteArray updateFinal();

    private:
        EVP_CIPHER_CTX _context;
    };

    // CipherPtr
    using CipherPtr = QSharedPointer<Cipher>;

    // Digest
    class Digest : public Base
    {
        Q_DISABLE_COPY(Digest)

        friend class Factory;

    private:
        Digest(const EVP_MD_CTX &context);

    public:
        virtual ~Digest() { EVP_MD_CTX_cleanup(&_context); }

        void update(const QByteArray &data);
        QByteArray updateFinal();

    private:
        EVP_MD_CTX _context;
    };

    // DigestPtr
    using DigestPtr = QSharedPointer<Digest>;

    // Signer
    class Signer : public Base
    {
        Q_DISABLE_COPY(Signer)

        friend class Factory;

    private:
        Signer(const EVP_MD_CTX &context, const KeyPtr &key);

    public:
        virtual ~Signer() { EVP_MD_CTX_cleanup(&_context); }

        void update(const QByteArray &data);
        QByteArray updateFinal();

    private:
        EVP_MD_CTX _context;
        KeyPtr _key;
    };

    // SignerPtr
    typedef QSharedPointer<Signer> SignerPtr;

    // Factory
    class Factory : public Base
    {
        Q_DISABLE_COPY(Factory)

    public:
        enum class SHA {
            SHA256,
            SHA512
        };

    private:
        Factory();
        virtual ~Factory();

    public:
        static Factory& instance();

        static QByteArray hash(const QByteArray &data, const Factory::SHA sha = Factory::SHA::SHA512);
        static QByteArray sign(const QByteArray &data, const QString &password);
        static QByteArray encrypt(const QByteArray &data, const QString &password);
        static QByteArray decrypt(const QByteArray &data, const QString &password);

        CipherPtr createCipher(const QString &password, const bool encrypt = true);
        DigestPtr createDigest(const Factory::SHA sha = Factory::SHA::SHA512);
        SignerPtr createSigner(const QString &password);
    };
}

#endif // CRYPTO_H
