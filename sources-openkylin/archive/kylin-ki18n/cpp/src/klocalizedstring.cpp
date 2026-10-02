// This file is part of the KDE libraries
// SPDX-FileCopyrightText: 2024 KI18n Rust Backend
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "KLocalizedString/klocalizedstring.h"
#include "KLocalizedString/kuitmarkup.h"

#include <QByteArray>
#include <QDebug>
#include <QFileInfo>
#include <QSet>
#include <QVector>

namespace {
/// 与上游 `KuitFormatterPrivate::format` 一致：在 Rust 侧使用 KUIT `tag_patterns` 前先执行
/// `setupForDomain`，否则 Rust 注册表为空，TermText 会误走 ANSI 回退。
void ensureKuitSetupForCurrentMessageDomains(const char *explicitDomain)
{
    Kuit::setupForDomain(KLocalizedString::applicationDomain());
    if (explicitDomain && explicitDomain[0] != '\0') {
        Kuit::setupForDomain(QByteArray(explicitDomain));
    }
}
} // namespace

extern "C" {

struct Ki18nRsMsgHandle;

Ki18nRsMsgHandle *ki18n_rs_msg_create(const char *context,
                                      const char *text,
                                      const char *plural);
Ki18nRsMsgHandle *ki18n_rs_msg_create_markup(const char *context,
                                             const char *text,
                                             const char *plural);
Ki18nRsMsgHandle *ki18n_rs_msg_clone(const Ki18nRsMsgHandle *handle);
void ki18n_rs_msg_destroy(Ki18nRsMsgHandle *handle);

int ki18n_rs_msg_set_domain(Ki18nRsMsgHandle *handle, const char *domain);
int ki18n_rs_msg_set_languages(Ki18nRsMsgHandle *handle,
                               const char *const *langs,
                               size_t count);
int ki18n_rs_msg_set_format(Ki18nRsMsgHandle *handle, int visual_format);
int ki18n_rs_msg_add_context(Ki18nRsMsgHandle *handle,
                              const char *key,
                              const char *value);
int ki18n_rs_msg_set_relax_subs(Ki18nRsMsgHandle *handle, int enabled);
int ki18n_rs_msg_set_ignore_markup(Ki18nRsMsgHandle *handle, int enabled);

int ki18n_rs_msg_subs_i32(Ki18nRsMsgHandle *handle,
                          qint32 value,
                          int field_width,
                          int base,
                          unsigned int fill_char);
int ki18n_rs_msg_subs_u32(Ki18nRsMsgHandle *handle,
                          quint32 value,
                          int field_width,
                          int base,
                          unsigned int fill_char);
int ki18n_rs_msg_subs_i64(Ki18nRsMsgHandle *handle,
                          qint64 value,
                          int field_width,
                          int base,
                          unsigned int fill_char);
int ki18n_rs_msg_subs_u64(Ki18nRsMsgHandle *handle,
                          quint64 value,
                          int field_width,
                          int base,
                          unsigned int fill_char);
int ki18n_rs_msg_subs_f64(Ki18nRsMsgHandle *handle,
                          double value,
                          int field_width,
                          char format,
                          int precision,
                          unsigned int fill_char);
int ki18n_rs_msg_subs_qstring(Ki18nRsMsgHandle *handle,
                              const char *utf8,
                              int field_width,
                              unsigned int fill_char);
int ki18n_rs_msg_subs_char(Ki18nRsMsgHandle *handle,
                           unsigned int char_utf32,
                           int field_width,
                           unsigned int fill_char);
int ki18n_rs_msg_subs_message(Ki18nRsMsgHandle *handle,
                              const Ki18nRsMsgHandle *other,
                              int field_width,
                              unsigned int fill_char);

int ki18n_rs_msg_to_string(const Ki18nRsMsgHandle *handle, char **out_utf8);
int ki18n_rs_msg_to_string_with_langs(const Ki18nRsMsgHandle *handle,
                                      const char *const *langs,
                                      size_t count,
                                      char **out_utf8);
int ki18n_rs_msg_to_string_with_domain(const Ki18nRsMsgHandle *handle,
                                       const char *domain,
                                       char **out_utf8);
int ki18n_rs_msg_get_untranslated_text(const Ki18nRsMsgHandle *handle, char **out_utf8);

void ki18n_rs_string_free(char *ptr);

int ki18n_rs_qt_domain_insert(const char *domain);
int ki18n_rs_qt_domain_remove(const char *domain);
int ki18n_rs_translate_qt(const char *context,
                          const char *source,
                          const char *comment,
                          int n,
                          char **out_utf8);

int ki18n_rs_set_application_domain(const char *domain);
int ki18n_rs_get_application_domain(char **out_domain);
int ki18n_rs_set_languages(const char *const *langs, size_t count);
int ki18n_rs_get_languages(char ***out_langs, size_t *out_count);
int ki18n_rs_clear_languages();
int ki18n_rs_add_domain_locale_dir(const char *domain, const char *path);
int ki18n_rs_available_languages(const char *domain, char ***out_langs, size_t *out_count);
int ki18n_rs_is_language_available(const char *domain, const char *language);
void ki18n_rs_string_array_free(char **arr, size_t count);

} // extern "C"

// -----------------------------------------------------------------------------
// Private implementation
// -----------------------------------------------------------------------------

class KLocalizedStringPrivate
{
public:
    explicit KLocalizedStringPrivate(Ki18nRsMsgHandle *h = nullptr)
        : handle(h)
    {
    }

    ~KLocalizedStringPrivate()
    {
        reset(nullptr);
    }

    void reset(Ki18nRsMsgHandle *h)
    {
        if (handle) {
            ki18n_rs_msg_destroy(handle);
        }
        handle = h;
    }

    Ki18nRsMsgHandle *cloneHandle() const
    {
        return handle ? ki18n_rs_msg_clone(handle) : nullptr;
    }

    Ki18nRsMsgHandle *handle = nullptr;
};

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

static QString fromRustString(char *ptr)
{
    if (!ptr) {
        return QString();
    }
    QString out = QString::fromUtf8(ptr);
    ki18n_rs_string_free(ptr);
    return out;
}

static QVector<QByteArray> toUtf8Vector(const QStringList &list, QVector<const char *> &outPtrs)
{
    QVector<QByteArray> storage;
    storage.reserve(list.size());
    outPtrs.reserve(list.size());
    for (const QString &s : list) {
        storage.push_back(s.toUtf8());
        outPtrs.push_back(storage.back().constData());
    }
    return storage;
}

// -----------------------------------------------------------------------------
// KLocalizedString core methods
// -----------------------------------------------------------------------------

KLocalizedString::KLocalizedString()
    : d(new KLocalizedStringPrivate())
{
}

KLocalizedString::KLocalizedString(const KLocalizedString &rhs)
    : d(new KLocalizedStringPrivate(rhs.d ? rhs.d->cloneHandle() : nullptr))
{
}

KLocalizedString &KLocalizedString::operator=(const KLocalizedString &rhs)
{
    if (this == &rhs) {
        return *this;
    }
    if (rhs.d) {
        d->reset(rhs.d->cloneHandle());
    } else {
        d->reset(nullptr);
    }
    return *this;
}

KLocalizedString::~KLocalizedString() = default;

bool KLocalizedString::isEmpty() const
{
    return !d || !d->handle;
}

QString KLocalizedString::toString() const
{
    if (isEmpty()) {
        return QString();
    }
    ensureKuitSetupForCurrentMessageDomains(nullptr);
    char *out = nullptr;
    const int rc = ki18n_rs_msg_to_string(d->handle, &out);
    if (rc != 0) {
        if (out) {
            ki18n_rs_string_free(out);
        }
        return QString();
    }
    return fromRustString(out);
}

QString KLocalizedString::toString(const QStringList &languages) const
{
    if (isEmpty()) {
        return QString();
    }
    if (languages.isEmpty()) {
        return toString();
    }

    QVector<const char *> ptrs;
    QVector<QByteArray> storage = toUtf8Vector(languages, ptrs);
    Q_UNUSED(storage);

    ensureKuitSetupForCurrentMessageDomains(nullptr);
    char *out = nullptr;
    const int rc = ki18n_rs_msg_to_string_with_langs(d->handle, ptrs.constData(), size_t(ptrs.size()), &out);
    if (rc != 0) {
        if (out) {
            ki18n_rs_string_free(out);
        }
        return QString();
    }
    return fromRustString(out);
}

QString KLocalizedString::toString(const char *domain) const
{
    if (isEmpty() || !domain) {
        return toString();
    }
    ensureKuitSetupForCurrentMessageDomains(domain);
    char *out = nullptr;
    const int rc = ki18n_rs_msg_to_string_with_domain(d->handle, domain, &out);
    if (rc != 0) {
        if (out) {
            ki18n_rs_string_free(out);
        }
        return QString();
    }
    return fromRustString(out);
}

QString KLocalizedString::toString(Kuit::VisualFormat format) const
{
    if (isEmpty()) {
        return QString();
    }

    // Do not modify this instance; work on a clone.
    Ki18nRsMsgHandle *clone = d->cloneHandle();
    if (!clone) {
        return QString();
    }
    const int rcFormat = ki18n_rs_msg_set_format(clone, int(format));
    if (rcFormat != 0) {
        ki18n_rs_msg_destroy(clone);
        return QString();
    }
    ensureKuitSetupForCurrentMessageDomains(nullptr);
    char *out = nullptr;
    const int rc = ki18n_rs_msg_to_string(clone, &out);
    ki18n_rs_msg_destroy(clone);
    if (rc != 0) {
        if (out) {
            ki18n_rs_string_free(out);
        }
        return QString();
    }
    return fromRustString(out);
}

KLocalizedString KLocalizedString::withLanguages(const QStringList &languages) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    if (languages.isEmpty()) {
        return copy;
    }
    QVector<const char *> ptrs;
    QVector<QByteArray> storage = toUtf8Vector(languages, ptrs);
    Q_UNUSED(storage);
    Q_ASSERT(copy.d && copy.d->handle);
    ki18n_rs_msg_set_languages(copy.d->handle, ptrs.constData(), size_t(ptrs.size()));
    return copy;
}

KLocalizedString KLocalizedString::withDomain(const char *domain) const
{
    if (isEmpty() || !domain) {
        return *this;
    }
    KLocalizedString copy(*this);
    if (!copy.d || !copy.d->handle) {
        return copy;
    }
    ki18n_rs_msg_set_domain(copy.d->handle, domain);
    return copy;
}

KLocalizedString KLocalizedString::withFormat(Kuit::VisualFormat format) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    if (!copy.d || !copy.d->handle) {
        return copy;
    }
    ki18n_rs_msg_set_format(copy.d->handle, int(format));
    return copy;
}

// -----------------------------------------------------------------------------
// subs overloads
// -----------------------------------------------------------------------------

KLocalizedString KLocalizedString::subs(int a, int fieldWidth, int base, QChar fillChar) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_subs_i32(copy.d->handle, a, fieldWidth, base, uint(fillChar.unicode()));
    return copy;
}

KLocalizedString KLocalizedString::subs(uint a, int fieldWidth, int base, QChar fillChar) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_subs_u32(copy.d->handle, a, fieldWidth, base, uint(fillChar.unicode()));
    return copy;
}

KLocalizedString KLocalizedString::subs(long a, int fieldWidth, int base, QChar fillChar) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_subs_i64(copy.d->handle, qint64(a), fieldWidth, base, uint(fillChar.unicode()));
    return copy;
}

KLocalizedString KLocalizedString::subs(ulong a, int fieldWidth, int base, QChar fillChar) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_subs_u64(copy.d->handle, quint64(a), fieldWidth, base, uint(fillChar.unicode()));
    return copy;
}

KLocalizedString KLocalizedString::subs(qlonglong a, int fieldWidth, int base, QChar fillChar) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_subs_i64(copy.d->handle, a, fieldWidth, base, uint(fillChar.unicode()));
    return copy;
}

KLocalizedString KLocalizedString::subs(qulonglong a, int fieldWidth, int base, QChar fillChar) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_subs_u64(copy.d->handle, a, fieldWidth, base, uint(fillChar.unicode()));
    return copy;
}

KLocalizedString KLocalizedString::subs(double a, int fieldWidth, char format, int precision, QChar fillChar) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_subs_f64(copy.d->handle, a, fieldWidth, format, precision, uint(fillChar.unicode()));
    return copy;
}

KLocalizedString KLocalizedString::subs(QChar a, int fieldWidth, QChar fillChar) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_subs_char(copy.d->handle, uint(a.unicode()), fieldWidth, uint(fillChar.unicode()));
    return copy;
}

KLocalizedString KLocalizedString::subs(const QString &a, int fieldWidth, QChar fillChar) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    const QByteArray utf8 = a.toUtf8();
    ki18n_rs_msg_subs_qstring(copy.d->handle, utf8.constData(), fieldWidth, uint(fillChar.unicode()));
    return copy;
}

KLocalizedString KLocalizedString::subs(const KLocalizedString &a, int fieldWidth, QChar fillChar) const
{
    if (isEmpty() || a.isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_subs_message(copy.d->handle,
                              a.d ? a.d->handle : nullptr,
                              fieldWidth,
                              uint(fillChar.unicode()));
    return copy;
}

// -----------------------------------------------------------------------------
// Misc helpers
// -----------------------------------------------------------------------------

KLocalizedString KLocalizedString::inContext(const QString &key, const QString &value) const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    const QByteArray k = key.toUtf8();
    const QByteArray v = value.toUtf8();
    ki18n_rs_msg_add_context(copy.d->handle, k.constData(), v.constData());
    return copy;
}

KLocalizedString KLocalizedString::relaxSubs() const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_set_relax_subs(copy.d->handle, 1);
    return copy;
}

KLocalizedString KLocalizedString::ignoreMarkup() const
{
    if (isEmpty()) {
        return *this;
    }
    KLocalizedString copy(*this);
    ki18n_rs_msg_set_ignore_markup(copy.d->handle, 1);
    return copy;
}

QByteArray KLocalizedString::untranslatedText() const
{
    if (isEmpty()) {
        return QByteArray();
    }
    char *out = nullptr;
    const int rc = ki18n_rs_msg_get_untranslated_text(d->handle, &out);
    if (rc != 0) {
        if (out) {
            ki18n_rs_string_free(out);
        }
        return QByteArray();
    }
    QByteArray result(out);
    ki18n_rs_string_free(out);
    return result;
}

// -----------------------------------------------------------------------------
// Private constructor used by ki18n* helpers
// -----------------------------------------------------------------------------

KLocalizedString::KLocalizedString(const char *domain,
                                   const char *context,
                                   const char *text,
                                   const char *plural,
                                   bool markupAware)
    : d(new KLocalizedStringPrivate())
{
    if (!text) {
        return;
    }
    Ki18nRsMsgHandle *handle =
        markupAware ? ki18n_rs_msg_create_markup(context, text, plural)
                    : ki18n_rs_msg_create(context, text, plural);
    d->reset(handle);
    if (domain && handle) {
        ki18n_rs_msg_set_domain(handle, domain);
    }
}

// -----------------------------------------------------------------------------
// Factory helpers (ki18n*/kxi18n*)
// -----------------------------------------------------------------------------

KLocalizedString ki18n(const char *text)
{
    return KLocalizedString(nullptr, nullptr, text, nullptr, false);
}

KLocalizedString ki18nc(const char *context, const char *text)
{
    return KLocalizedString(nullptr, context, text, nullptr, false);
}

KLocalizedString ki18np(const char *singular, const char *plural)
{
    return KLocalizedString(nullptr, nullptr, singular, plural, false);
}

KLocalizedString ki18ncp(const char *context, const char *singular, const char *plural)
{
    return KLocalizedString(nullptr, context, singular, plural, false);
}

KLocalizedString ki18nd(const char *domain, const char *text)
{
    return KLocalizedString(domain, nullptr, text, nullptr, false);
}

KLocalizedString ki18ndc(const char *domain, const char *context, const char *text)
{
    return KLocalizedString(domain, context, text, nullptr, false);
}

KLocalizedString ki18ndp(const char *domain, const char *singular, const char *plural)
{
    return KLocalizedString(domain, nullptr, singular, plural, false);
}

KLocalizedString ki18ndcp(const char *domain, const char *context, const char *singular, const char *plural)
{
    return KLocalizedString(domain, context, singular, plural, false);
}

KLocalizedString kxi18n(const char *text)
{
    return KLocalizedString(nullptr, nullptr, text, nullptr, true);
}

KLocalizedString kxi18nc(const char *context, const char *text)
{
    return KLocalizedString(nullptr, context, text, nullptr, true);
}

KLocalizedString kxi18np(const char *singular, const char *plural)
{
    return KLocalizedString(nullptr, nullptr, singular, plural, true);
}

KLocalizedString kxi18ncp(const char *context, const char *singular, const char *plural)
{
    return KLocalizedString(nullptr, context, singular, plural, true);
}

KLocalizedString kxi18nd(const char *domain, const char *text)
{
    return KLocalizedString(domain, nullptr, text, nullptr, true);
}

KLocalizedString kxi18ndc(const char *domain, const char *context, const char *text)
{
    return KLocalizedString(domain, context, text, nullptr, true);
}

KLocalizedString kxi18ndp(const char *domain, const char *singular, const char *plural)
{
    return KLocalizedString(domain, nullptr, singular, plural, true);
}

KLocalizedString kxi18ndcp(const char *domain, const char *context, const char *singular, const char *plural)
{
    return KLocalizedString(domain, context, singular, plural, true);
}

// 与上游 ki18n/src/i18n/common_helpers.cpp::removeAcceleratorMarker 对齐。
static QString removeReducedCJKAccMark(const QString &label, int pos)
{
    if (pos > 0 && pos + 1 < label.length() //
        && label[pos - 1] == QLatin1Char('(') //
        && label[pos + 1] == QLatin1Char(')') //
        && label[pos].isLetterOrNumber()) {
        int len = label.length();
        int p1 = pos - 2;
        while (p1 >= 0 && !label[p1].isLetterOrNumber()) {
            --p1;
        }
        ++p1;
        int p2 = pos + 2;
        while (p2 < len && !label[p2].isLetterOrNumber()) {
            ++p2;
        }
        --p2;

        // Qt5 无 QStringView + QStringView；直接用 QString 切片。
        if (p1 == 0) {
            return label.left(pos - 1) + label.mid(p2 + 1);
        }
        if (p2 + 1 == len) {
            return label.left(p1) + label.mid(pos + 2);
        }
    }
    return label;
}

static QString removeAcceleratorMarkerImpl(const QString &label_)
{
    QString label = label_;

    int p = 0;
    bool accmarkRemoved = false;
    while (true) {
        p = label.indexOf(QLatin1Char('&'), p);
        if (p < 0 || p + 1 == label.length()) {
            break;
        }

        const QChar marker = label.at(p + 1);
        if (marker.isLetterOrNumber()) {
            label = label.left(p) + label.mid(p + 1);
            label = removeReducedCJKAccMark(label, p);
            accmarkRemoved = true;
        } else if (marker == QLatin1Char('&')) {
            label = label.left(p) + label.mid(p + 1);
        }

        ++p;
    }

    if (!accmarkRemoved) {
        bool hasCJK = false;
        for (const QChar c : label) {
            if (c.unicode() >= 0x2e00) {
                hasCJK = true;
                break;
            }
        }
        if (hasCJK) {
            p = 0;
            while (true) {
                p = label.indexOf(QLatin1Char('('), p);
                if (p < 0) {
                    break;
                }
                label = removeReducedCJKAccMark(label, p + 1);
                ++p;
            }
        }
    }

    return label;
}

void KLocalizedString::setApplicationDomain(const char *domain)
{
    ki18n_rs_set_application_domain(domain);
}

QByteArray KLocalizedString::applicationDomain()
{
    char *out = nullptr;
    const int rc = ki18n_rs_get_application_domain(&out);
    if (rc != 0 || !out) {
        return QByteArray();
    }
    QByteArray result = QByteArray(out);
    ki18n_rs_string_free(out);
    return result;
}

QStringList KLocalizedString::languages()
{
    char **langs = nullptr;
    size_t count = 0;
    const int rc = ki18n_rs_get_languages(&langs, &count);
    if (rc != 0 || !langs) {
        return QStringList();
    }
    QStringList result;
    for (size_t i = 0; i < count; ++i) {
        if (langs[i]) {
            result.append(QString::fromUtf8(langs[i]));
        }
    }
    ki18n_rs_string_array_free(langs, count);
    return result;
}

void KLocalizedString::setLanguages(const QStringList &languages)
{
    QVector<const char *> ptrs;
    QVector<QByteArray> storage = toUtf8Vector(languages, ptrs);
    Q_UNUSED(storage);
    ki18n_rs_set_languages(ptrs.constData(), ptrs.size());
}

void KLocalizedString::clearLanguages()
{
    ki18n_rs_clear_languages();
}

bool KLocalizedString::isApplicationTranslatedInto(const QString &language)
{
    const QByteArray domain = applicationDomain();
    if (domain.isEmpty()) {
        return false;
    }

    const QByteArray langUtf8 = language.toUtf8();
    return ki18n_rs_is_language_available(domain.constData(), langUtf8.constData()) != 0;
}

QSet<QString> KLocalizedString::availableApplicationTranslations()
{
    return availableDomainTranslations(applicationDomain());
}

QSet<QString> KLocalizedString::availableDomainTranslations(const QByteArray &domain)
{
    QSet<QString> result;
    if (domain.isEmpty()) {
        return result;
    }
    char **langs = nullptr;
    size_t count = 0;
    const int rc = ki18n_rs_available_languages(domain.constData(), &langs, &count);
    if (rc != 0 || !langs) {
        if (langs) {
            ki18n_rs_string_array_free(langs, count);
        }
        return result;
    }
    for (size_t i = 0; i < count; ++i) {
        if (langs[i]) {
            result.insert(QString::fromUtf8(langs[i]));
        }
    }
    ki18n_rs_string_array_free(langs, count);
    result.insert(QString::fromUtf8("en_US"));
    return result;
}

void KLocalizedString::addDomainLocaleDir(const QByteArray &domain, const QString &path)
{
    ki18n_rs_add_domain_locale_dir(domain.constData(), path.toUtf8().constData());
}

QString KLocalizedString::localizedFilePath(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    const QString locDirPath = fileInfo.path() + QLatin1Char('/') + QLatin1String("l10n");
    QFileInfo locDirInfo(locDirPath);
    if (!locDirInfo.isDir()) {
        return filePath;
    }

    const QString fileName = fileInfo.fileName();
    const QStringList langs = languages();
    for (const QString &lang : langs) {
        const QString locFilePath = locDirPath + QLatin1Char('/') + lang + QLatin1Char('/') + fileName;
        QFileInfo locFileInfo(locFilePath);
        if (locFileInfo.isFile() && locFileInfo.isReadable()) {
            return locFilePath;
        }
    }

    return filePath;
}

QString KLocalizedString::removeAcceleratorMarker(const QString &label)
{
    return removeAcceleratorMarkerImpl(label);
}

QString KLocalizedString::translateQt(const char *context, const char *sourceText, const char *comment, int n)
{
    if (!sourceText || !sourceText[0]) {
        qWarning() << "KLocalizedString::translateQt: Trying to look up translation of \"\", fix the code.";
        return QString();
    }
    char *out = nullptr;
    const int rc = ki18n_rs_translate_qt(context, sourceText, comment, n, &out);
    if (rc != 0) {
        if (out) {
            ki18n_rs_string_free(out);
        }
        return QString();
    }
    return fromRustString(out);
}

void KLocalizedString::insertQtDomain(const char *domain)
{
    if (!domain) {
        return;
    }
    ki18n_rs_qt_domain_insert(domain);
}

void KLocalizedString::removeQtDomain(const char *domain)
{
    if (!domain) {
        return;
    }
    ki18n_rs_qt_domain_remove(domain);
}

