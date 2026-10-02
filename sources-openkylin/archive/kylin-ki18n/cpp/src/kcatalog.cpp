// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "KLocalizedString/kcatalog.h"

#include <QCoreApplication>
#include <QDir>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QStandardPaths>
#include <QStringList>

extern "C" {

// Global/catalog-level FFI exported by kylin-ki18n
int ki18n_rs_catalog_translate(const char *domain,
                               const char *msgid,
                               const char *msgid_plural,
                               long long n,
                               const char *context,
                               char **out_utf8);

int ki18n_rs_available_languages(const char *domain,
                                 char ***out_langs,
                                 size_t *out_count);

int ki18n_rs_catalog_locale_dir(const char *domain,
                                const char *lang,
                                char **out_path);

void ki18n_rs_string_free(char *ptr);
void ki18n_rs_string_array_free(char **arr, size_t count);

} // extern "C"

// -----------------------------------------------------------------------------
// Private data
// -----------------------------------------------------------------------------

class KCatalogPrivate : public QSharedData
{
public:
    KCatalogPrivate()
        : bindDone(false)
    {
    }

    QByteArray domain;
    QByteArray language;
    QByteArray localeDir;
    bool bindDone;
};

class KCatalogStaticData
{
public:
    KCatalogStaticData() {}
    QHash<QByteArray, QString> customCatalogDirs;
    QMutex mutex;
};

Q_GLOBAL_STATIC(KCatalogStaticData, catalogStaticData)

// -----------------------------------------------------------------------------
// KCatalog implementation
// -----------------------------------------------------------------------------

KCatalog::KCatalog()
    : d(new KCatalogPrivate)
{
}

KCatalog::KCatalog(const QByteArray &domain)
    : d(new KCatalogPrivate)
{
    d->domain = domain;
}

KCatalog::KCatalog(const QByteArray &domain, const QString &language_)
    : d(new KCatalogPrivate)
{
    d->domain = domain;
    d->language = language_.toUtf8();
    d->localeDir = catalogLocaleDir(domain, language_).toUtf8();
}

KCatalog::KCatalog(const KCatalog &) = default;
KCatalog &KCatalog::operator=(const KCatalog &) = default;
KCatalog::~KCatalog() = default;

bool KCatalog::isNull() const
{
    return !d || d->domain.isEmpty();
}

QString KCatalog::translate(const QByteArray &msgid,
                            const QByteArray &msgidPlural,
                            qint64 n,
                            const QByteArray &context) const
{
    if (isNull() || msgid.isEmpty()) {
        return QString();
    }

    const char *pluralPtr = msgidPlural.isEmpty() ? nullptr : msgidPlural.constData();
    const char *ctxPtr = context.isEmpty() ? nullptr : context.constData();

    char *out = nullptr;
    const int rc = ki18n_rs_catalog_translate(d->domain.constData(),
                                              msgid.constData(),
                                              pluralPtr,
                                              static_cast<long long>(n),
                                              ctxPtr,
                                              &out);
    if (rc != 0) {
        if (out) {
            ki18n_rs_string_free(out);
        }
        return QString();
    }

    QString result = out ? QString::fromUtf8(out) : QString();
    if (out) {
        ki18n_rs_string_free(out);
    }
    return result;
}

QString KCatalog::localeDir() const
{
    if (isNull()) {
        return QString();
    }

    // Discover available languages for this domain
    char **langs = nullptr;
    size_t count = 0;
    QString localeDir;

    int rcLangs = ki18n_rs_available_languages(d->domain.constData(), &langs, &count);
    if (rcLangs != 0 || !langs || count == 0) {
        if (langs) {
            ki18n_rs_string_array_free(langs, count);
        }
        return localeDir;
    }

    // Use the first available language as a reasonable default
    QByteArray langUtf8(langs[0]);

    char *path = nullptr;
    int rcPath = ki18n_rs_catalog_locale_dir(d->domain.constData(), langUtf8.constData(), &path);
    if (rcPath == 0 && path) {
        localeDir = QString::fromUtf8(path);
    }
    if (path) {
        ki18n_rs_string_free(path);
    }

    ki18n_rs_string_array_free(langs, count);
    return localeDir;
}

QStringList KCatalog::languages() const
{
    QStringList result;
    if (isNull()) {
        return result;
    }

    char **langs = nullptr;
    size_t count = 0;
    int rc = ki18n_rs_available_languages(d->domain.constData(), &langs, &count);
    if (rc != 0 || !langs) {
        if (langs) {
            ki18n_rs_string_array_free(langs, count);
        }
        return result;
    }

    for (size_t i = 0; i < count; ++i) {
        if (langs[i]) {
            result.append(QString::fromUtf8(langs[i]));
        }
    }

    ki18n_rs_string_array_free(langs, count);
    return result;
}

QString KCatalog::translate(const QByteArray &msgid) const
{
    return translate(msgid, QByteArray(), 0, QByteArray());
}

QString KCatalog::translate(const QByteArray &msgctxt, const QByteArray &msgid) const
{
    return translate(msgid, QByteArray(), 0, msgctxt);
}

QString KCatalog::translate(const QByteArray &msgid, const QByteArray &msgid_plural, qulonglong n) const
{
    return translate(msgid, msgid_plural, n, QByteArray());
}

QString KCatalog::translate(const QByteArray &msgctxt, const QByteArray &msgid, 
                             const QByteArray &msgid_plural, qulonglong n) const
{
    return translate(msgid, msgid_plural, n, msgctxt);
}

QString KCatalog::catalogLocaleDir(const QByteArray &domain, const QString &language)
{
    QString relpath = QStringLiteral("%1/LC_MESSAGES/%2.mo").arg(language, QString::fromUtf8(domain));

    {
        QMutexLocker lock(&catalogStaticData->mutex);
        const QString customLocaleDir = catalogStaticData->customCatalogDirs.value(domain);
        if (!customLocaleDir.isEmpty()) {
            return customLocaleDir;
        }
    }

    QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("locale/") + relpath);
    if (file.isEmpty()) {
        return QString();
    }

    QString localeDir = file.left(file.size() - relpath.size());
    int lastSlash = localeDir.lastIndexOf(QLatin1Char('/'));
    if (lastSlash >= 0) {
        localeDir = localeDir.left(lastSlash);
    }
    return localeDir;
}

QSet<QString> KCatalog::availableCatalogLanguages(const QByteArray &domain_)
{
    QString domain = QString::fromUtf8(domain_);
    QStringList localeDirPaths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("locale"), QStandardPaths::LocateDirectory);

    {
        QMutexLocker lock(&catalogStaticData->mutex);
        auto it = catalogStaticData->customCatalogDirs.constFind(domain_);
        if (it != catalogStaticData->customCatalogDirs.constEnd()) {
            localeDirPaths.prepend(*it);
        }
    }

    QSet<QString> availableLanguages;
    for (const QString &localDirPath : localeDirPaths) {
        QDir localeDir(localDirPath);
        const QStringList languages = localeDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        for (const QString &language : languages) {
            QString relPath = QStringLiteral("%1/LC_MESSAGES/%2.mo").arg(language, domain);
            if (localeDir.exists(relPath)) {
                availableLanguages.insert(language);
            }
        }
    }
    return availableLanguages;
}

void KCatalog::addDomainLocaleDir(const QByteArray &domain, const QString &path)
{
    QMutexLocker lock(&catalogStaticData->mutex);
    catalogStaticData->customCatalogDirs.insert(domain, path);
}

