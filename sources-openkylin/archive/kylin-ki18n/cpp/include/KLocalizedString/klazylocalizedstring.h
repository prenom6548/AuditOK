/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLAZYLOCALIZEDSTRING_H
#define KLAZYLOCALIZEDSTRING_H

#include "klocalizedstring.h"

#include <cstddef>

/**
 *  @class KLazyLocalizedString klazylocalizedstring.h <KLazyLocalizedString>
 *
 *  Lazy-initialized variant of KLocalizedString.
 *
 *  This is a safer replacement for the I18N_NOOP set of macros and allows
 *  marking strings for extraction without runtime-initializing KLocalizedString instances,
 *  for storing in static data tables.
 *
 *  Instances of KLazyLocalizedString are not created directly, unless they should be empty.
 *  Instead they have to be obtained via the kli18n* functions (similar to KLocalizedString
 *  and the ki18n* functions).
 */
class KLazyLocalizedString
{
public:
    constexpr inline KLazyLocalizedString() = default;

    Q_REQUIRED_RESULT inline operator KLocalizedString() const
    {
        if (!m_text) {
            return KLocalizedString();
        }
#ifdef TRANSLATION_DOMAIN
        return KLocalizedString(TRANSLATION_DOMAIN, m_context, m_text, m_plural, m_markupAware);
#else
        return KLocalizedString(nullptr, m_context, m_text, m_plural, m_markupAware);
#endif
    }

    Q_REQUIRED_RESULT constexpr inline bool isEmpty() const
    {
        return (m_text == nullptr) || (m_text[0] == '\0');
    }

    Q_REQUIRED_RESULT constexpr inline const char *untranslatedText() const
    {
        return m_text;
    }

    Q_REQUIRED_RESULT inline QString toString() const
    {
        return this->operator KLocalizedString().toString();
    }

    Q_REQUIRED_RESULT inline QString toString(const QStringList &languages) const
    {
        return this->operator KLocalizedString().toString(languages);
    }

    Q_REQUIRED_RESULT inline QString toString(const char *domain) const
    {
        return this->operator KLocalizedString().toString(domain);
    }

    Q_REQUIRED_RESULT inline QString toString(Kuit::VisualFormat format) const
    {
        return this->operator KLocalizedString().toString(format);
    }

    Q_REQUIRED_RESULT inline KLocalizedString withLanguages(const QStringList &languages) const
    {
        return this->operator KLocalizedString().withLanguages(languages);
    }

    Q_REQUIRED_RESULT inline KLocalizedString withDomain(const char *domain) const
    {
        return this->operator KLocalizedString().withDomain(domain);
    }

    Q_REQUIRED_RESULT inline KLocalizedString withFormat(Kuit::VisualFormat format) const
    {
        return this->operator KLocalizedString().withFormat(format);
    }

    Q_REQUIRED_RESULT inline KLocalizedString subs(int a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    Q_REQUIRED_RESULT inline KLocalizedString subs(uint a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    Q_REQUIRED_RESULT inline KLocalizedString subs(long a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    Q_REQUIRED_RESULT inline KLocalizedString subs(ulong a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    Q_REQUIRED_RESULT inline KLocalizedString subs(qlonglong a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    Q_REQUIRED_RESULT inline KLocalizedString subs(qulonglong a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    Q_REQUIRED_RESULT inline KLocalizedString subs(double a,
                                                   int fieldWidth = 0,
                                                   char format = 'g',
                                                   int precision = -1,
                                                   QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, format, precision, fillChar);
    }

    Q_REQUIRED_RESULT inline KLocalizedString subs(QChar a, int fieldWidth = 0, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, fillChar);
    }

    Q_REQUIRED_RESULT inline KLocalizedString subs(const QString &a, int fieldWidth = 0, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, fillChar);
    }

    Q_REQUIRED_RESULT inline KLocalizedString subs(const KLocalizedString &a, int fieldWidth = 0, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, fillChar);
    }

    Q_REQUIRED_RESULT inline KLocalizedString inContext(const QString &key, const QString &value) const
    {
        return this->operator KLocalizedString().inContext(key, value);
    }

    Q_REQUIRED_RESULT inline KLocalizedString relaxSubs() const
    {
        return this->operator KLocalizedString().relaxSubs();
    }

    Q_REQUIRED_RESULT inline KLocalizedString ignoreMarkup() const
    {
        return this->operator KLocalizedString().ignoreMarkup();
    }

private:
    template<std::size_t TextSize>
    friend inline constexpr KLazyLocalizedString kli18n(const char (&text)[TextSize]);
    template<std::size_t ContextSize, std::size_t TextSize>
    friend constexpr inline KLazyLocalizedString kli18nc(const char (&context)[ContextSize], const char (&text)[TextSize]);
    template<std::size_t SingularSize, std::size_t PluralSize>
    friend constexpr inline KLazyLocalizedString kli18np(const char (&singular)[SingularSize], const char (&plural)[PluralSize]);
    template<std::size_t ContextSize, std::size_t SingularSize, std::size_t PluralSize>
    friend constexpr inline KLazyLocalizedString
    kli18ncp(const char (&context)[ContextSize], const char (&singular)[SingularSize], const char (&plural)[PluralSize]);
    template<std::size_t TextSize>
    friend constexpr inline KLazyLocalizedString klxi18n(const char (&text)[TextSize]);
    template<std::size_t ContextSize, std::size_t TextSize>
    friend constexpr inline KLazyLocalizedString klxi18nc(const char (&context)[ContextSize], const char (&text)[TextSize]);
    template<std::size_t SingularSize, std::size_t PluralSize>
    friend constexpr inline KLazyLocalizedString klxi18np(const char (&singular)[SingularSize], const char (&plural)[PluralSize]);
    template<std::size_t ContextSize, std::size_t SingularSize, std::size_t PluralSize>
    friend constexpr inline KLazyLocalizedString
    klxi18ncp(const char (&context)[ContextSize], const char (&singular)[SingularSize], const char (&plural)[PluralSize]);

    constexpr inline KLazyLocalizedString(const char *context, const char *text, const char *plural, bool markupAware)
        : m_context(context)
        , m_text(text)
        , m_plural(plural)
        , m_markupAware(markupAware)
    {
    }

    const char *m_context = nullptr;
    const char *m_text = nullptr;
    const char *m_plural = nullptr;
    bool m_markupAware = false;
};

/**
 * Mark the string @p text for extraction.
 */
template<std::size_t TextSize>
constexpr inline KLazyLocalizedString kli18n(const char (&text)[TextSize])
{
    return KLazyLocalizedString(nullptr, text, nullptr, false);
}

/**
 * Mark the string @p text with @p context for extraction.
 */
template<std::size_t ContextSize, std::size_t TextSize>
constexpr inline KLazyLocalizedString kli18nc(const char (&context)[ContextSize], const char (&text)[TextSize])
{
    return KLazyLocalizedString(context, text, nullptr, false);
}

/**
 * Mark the string @p singular and @p plural for extraction.
 */
template<std::size_t SingularSize, std::size_t PluralSize>
constexpr inline KLazyLocalizedString kli18np(const char (&singular)[SingularSize], const char (&plural)[PluralSize])
{
    return KLazyLocalizedString(nullptr, singular, plural, false);
}

/**
 * Mark the string @p singular and @p plural with @p context for extraction.
 */
template<std::size_t ContextSize, std::size_t SingularSize, std::size_t PluralSize>
constexpr inline KLazyLocalizedString kli18ncp(const char (&context)[ContextSize],
                                               const char (&singular)[SingularSize],
                                               const char (&plural)[PluralSize])
{
    return KLazyLocalizedString(context, singular, plural, false);
}

/**
 * Mark the markup-aware string @p text for extraction.
 */
template<std::size_t TextSize>
constexpr inline KLazyLocalizedString klxi18n(const char (&text)[TextSize])
{
    return KLazyLocalizedString(nullptr, text, nullptr, true);
}

/**
 * Mark the markup-aware string @p text with @p context for extraction.
 */
template<std::size_t ContextSize, std::size_t TextSize>
constexpr inline KLazyLocalizedString klxi18nc(const char (&context)[ContextSize], const char (&text)[TextSize])
{
    return KLazyLocalizedString(context, text, nullptr, true);
}

/**
 * Mark the markup-aware string @p singular and @p plural for extraction.
 */
template<std::size_t SingularSize, std::size_t PluralSize>
constexpr inline KLazyLocalizedString klxi18np(const char (&singular)[SingularSize], const char (&plural)[PluralSize])
{
    return KLazyLocalizedString(nullptr, singular, plural, true);
}

/**
 * Mark the markup-aware string @p singular and @p plural with @p context for extraction.
 */
template<std::size_t ContextSize, std::size_t SingularSize, std::size_t PluralSize>
constexpr inline KLazyLocalizedString klxi18ncp(const char (&context)[ContextSize],
                                                const char (&singular)[SingularSize],
                                                const char (&plural)[PluralSize])
{
    return KLazyLocalizedString(context, singular, plural, true);
}

#endif // KLAZYLOCALIZEDSTRING_H

