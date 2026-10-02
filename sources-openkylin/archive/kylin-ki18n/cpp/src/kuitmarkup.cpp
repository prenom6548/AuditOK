// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "KLocalizedString/kuitmarkup.h"
#include "KLocalizedString/klocalizedstring.h"
#include "kuitsetup_p.h"

#include <QByteArray>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QtGlobal>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <unordered_map>

extern "C" {

void ki18n_rs_kuit_set_format(const char *domain,
                              int tagClass,
                              const char *tag,
                              int format,
                              const char *replacement);

void ki18n_rs_kuit_set_tooltip_attribute(const char *domain,
                                         const char *tag,
                                         const char *attribute);

void ki18n_rs_kuit_set_tag_pattern(const char *domain,
                                   const char *tag,
                                   const char *const *attrib_names,
                                   std::size_t attrib_count,
                                   int format,
                                   const char *pattern,
                                   int leading_newlines,
                                   std::uint64_t formatter_handle);

using Ki18nKuitFormatterInvoker = int (*)(std::uint64_t handle,
                                            const char *const *langs,
                                            std::size_t lang_count,
                                            const char *tag,
                                            const char *const *attr_keys,
                                            const char *const *attr_vals,
                                            std::size_t attr_count,
                                            const char *text,
                                            const char *const *tag_path,
                                            std::size_t tag_path_len,
                                            int visual_format,
                                            char **out);

void ki18n_rs_kuit_register_formatter_invoker(Ki18nKuitFormatterInvoker invoker);

void ki18n_rs_kuit_register_warning_handler(void (*handler)(const char *));

int ki18n_rs_dup_utf8(const unsigned char *ptr, std::size_t len, char **out);

} // extern "C"

namespace {

std::mutex s_formatter_mtx;
std::unordered_map<std::uint64_t, Kuit::TagFormatter> s_formatters;
std::uint64_t s_next_formatter_id = 1;

std::uint64_t register_formatter(Kuit::TagFormatter f)
{
    std::lock_guard<std::mutex> lock(s_formatter_mtx);
    const auto id = s_next_formatter_id++;
    s_formatters[id] = f;
    return id;
}

extern "C" int ki18n_cpp_kuit_invoke_formatter(std::uint64_t handle,
                                               const char *const *langs,
                                               std::size_t lang_count,
                                               const char *tag,
                                               const char *const *attr_keys,
                                               const char *const *attr_vals,
                                               std::size_t attr_count,
                                               const char *text,
                                               const char *const *tag_path,
                                               std::size_t tag_path_len,
                                               int visual_format,
                                               char **out)
{
    Kuit::TagFormatter fn = nullptr;
    {
        std::lock_guard<std::mutex> lock(s_formatter_mtx);
        const auto it = s_formatters.find(handle);
        if (it == s_formatters.end()) {
            return -1;
        }
        fn = it->second;
    }
    if (!fn || !out) {
        return -2;
    }

    QStringList qlangs;
    for (std::size_t i = 0; i < lang_count; ++i) {
        if (langs && langs[i]) {
            qlangs.append(QString::fromUtf8(langs[i]));
        }
    }

    const QString qtag = QString::fromUtf8(tag ? tag : "");
    QHash<QString, QString> attributes;
    for (std::size_t i = 0; i < attr_count; ++i) {
        if (attr_keys && attr_vals && attr_keys[i] && attr_vals[i]) {
            attributes.insert(QString::fromUtf8(attr_keys[i]), QString::fromUtf8(attr_vals[i]));
        }
    }

    const QString qtext = QString::fromUtf8(text ? text : "");
    QStringList path;
    for (std::size_t i = 0; i < tag_path_len; ++i) {
        if (tag_path && tag_path[i]) {
            path.append(QString::fromUtf8(tag_path[i]));
        }
    }

    const auto vf = static_cast<Kuit::VisualFormat>(visual_format);
    const QString result = fn(qlangs, qtag, attributes, qtext, path, vf);
    const QByteArray utf8 = result.toUtf8();
    return ki18n_rs_dup_utf8(reinterpret_cast<const unsigned char *>(utf8.constData()),
                             static_cast<std::size_t>(utf8.size()),
                             out);
}

struct RegisterKuitInvoker {
    RegisterKuitInvoker()
    {
        ki18n_rs_kuit_register_formatter_invoker(&ki18n_cpp_kuit_invoke_formatter);
    }
};

static RegisterKuitInvoker s_register_kuit_invoker;

extern "C" void ki18n_cpp_kuit_emit_warning(const char *msg)
{
    qWarning("%s", msg ? msg : "");
}

struct RegisterKuitWarningHandler {
    RegisterKuitWarningHandler()
    {
        ki18n_rs_kuit_register_warning_handler(&ki18n_cpp_kuit_emit_warning);
    }
};

static RegisterKuitWarningHandler s_register_kuit_warning_handler;

} // namespace

void KuitSetup::setTagPattern(const QString &tagName,
                              const QStringList &attribNames,
                              Kuit::VisualFormat format,
                              const KLocalizedString &pattern,
                              Kuit::TagFormatter formatter,
                              int leadingNewlines)
{
    const QByteArray tagUtf8 = tagName.toUtf8();

    QList<QByteArray> attribStorage;
    QVector<const char *> attribPtrs;
    attribStorage.reserve(attribNames.size());
    attribPtrs.reserve(attribNames.size());
    for (const QString &nm : attribNames) {
        if (nm.isEmpty()) {
            continue;
        }
        attribStorage.append(nm.toUtf8());
    }
    for (const QByteArray &b : attribStorage) {
        attribPtrs.append(b.constData());
    }

    const QByteArray patternUtf8 = pattern.untranslatedText();
    const char *patternPtr = patternUtf8.isEmpty() ? nullptr : patternUtf8.constData();

    std::uint64_t handle = 0;
    if (formatter) {
        handle = register_formatter(formatter);
    }

    const QByteArray domainUtf8 = d->domain;
    const char *domainPtr = domainUtf8.isEmpty() ? "" : domainUtf8.constData();

    ki18n_rs_kuit_set_tag_pattern(domainPtr,
                                  tagUtf8.constData(),
                                  attribPtrs.isEmpty() ? nullptr : attribPtrs.constData(),
                                  static_cast<std::size_t>(attribPtrs.size()),
                                  static_cast<int>(format),
                                  patternPtr,
                                  leadingNewlines,
                                  handle);
}

void KuitSetup::setTagClass(const QString &tagName, Kuit::TagClass aClass)
{
    const QByteArray tagUtf8 = tagName.toUtf8();
    const QByteArray empty;
    const QByteArray domainUtf8 = d->domain;
    const char *domainPtr = domainUtf8.isEmpty() ? "" : domainUtf8.constData();
    ki18n_rs_kuit_set_format(domainPtr,
                             static_cast<int>(aClass),
                             tagUtf8.constData(),
                             static_cast<int>(Kuit::VisualFormat::UndefinedFormat),
                             empty.constData());
}

void KuitSetup::setFormatForMarker(const QString &marker, Kuit::VisualFormat format)
{
    const QByteArray markerUtf8 = marker.toUtf8();
    const QByteArray empty;
    const QByteArray domainUtf8 = d->domain;
    const char *domainPtr = domainUtf8.isEmpty() ? "" : domainUtf8.constData();
    ki18n_rs_kuit_set_format(domainPtr,
                             static_cast<int>(Kuit::PhraseTag),
                             markerUtf8.constData(),
                             static_cast<int>(format),
                             empty.constData());
}
