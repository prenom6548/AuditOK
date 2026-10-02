// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "KLocalizedString/klocalizedcontext.h"

#include "KLocalizedString/klocalizedstring.h"

#include <QDebug>
#include <QMetaType>
#include <QVariant>

class KLocalizedContextPrivate
{
};

KLocalizedContext::KLocalizedContext(QObject *parent)
    : QObject(parent)
{
}

KLocalizedContext::~KLocalizedContext() = default;

static QString s_translationDomain;

QString KLocalizedContext::translationDomain() const
{
    return s_translationDomain;
}

void KLocalizedContext::setTranslationDomain(const QString &domain)
{
    if (domain != s_translationDomain) {
        s_translationDomain = domain;
        Q_EMIT translationDomainChanged(domain);
    }
}

static void subsVariant(KLocalizedString &trMessage, const QVariant &value)
{
    switch (value.userType()) {
    case QMetaType::QString:
        trMessage = trMessage.subs(value.toString());
        break;
    case QMetaType::Int:
        trMessage = trMessage.subs(value.toInt());
        break;
    case QMetaType::UInt:
        trMessage = trMessage.subs(static_cast<int>(value.toUInt()));
        break;
    case QMetaType::LongLong:
        trMessage = trMessage.subs(value.toLongLong());
        break;
    case QMetaType::ULongLong:
        trMessage = trMessage.subs(value.toULongLong());
        break;
    case QMetaType::Double:
        trMessage = trMessage.subs(value.toDouble());
        break;
    case QMetaType::Float:
        trMessage = trMessage.subs(static_cast<double>(value.toFloat()));
        break;
    case QMetaType::Bool:
        trMessage = trMessage.subs(value.toBool() ? 1 : 0);
        break;
    case QMetaType::QChar:
        trMessage = trMessage.subs(value.toChar());
        break;
    default:
        if (value.canConvert(QMetaType::QString)) {
            trMessage = trMessage.subs(value.toString());
        } else if (value.canConvert(QMetaType::Double)) {
            trMessage = trMessage.subs(value.toDouble());
        } else if (value.canConvert(QMetaType::LongLong)) {
            trMessage = trMessage.subs(value.toLongLong());
        } else {
            trMessage = trMessage.subs(QStringLiteral("???"));
            qWarning() << "couldn't convert" << value << "to translate";
        }
    }
}

static void resolveMessage(KLocalizedString &trMessage,
                           const QVariant &param1,
                           const QVariant &param2,
                           const QVariant &param3,
                           const QVariant &param4,
                           const QVariant &param5,
                           const QVariant &param6,
                           const QVariant &param7,
                           const QVariant &param8,
                           const QVariant &param9,
                           const QVariant &param10 = QVariant())
{
    if (!param1.isNull()) {
        subsVariant(trMessage, param1);
    }
    if (!param2.isNull()) {
        subsVariant(trMessage, param2);
    }
    if (!param3.isNull()) {
        subsVariant(trMessage, param3);
    }
    if (!param4.isNull()) {
        subsVariant(trMessage, param4);
    }
    if (!param5.isNull()) {
        subsVariant(trMessage, param5);
    }
    if (!param6.isNull()) {
        subsVariant(trMessage, param6);
    }
    if (!param7.isNull()) {
        subsVariant(trMessage, param7);
    }
    if (!param8.isNull()) {
        subsVariant(trMessage, param8);
    }
    if (!param9.isNull()) {
        subsVariant(trMessage, param9);
    }
    if (!param10.isNull()) {
        subsVariant(trMessage, param10);
    }
}

static void resolvePlural(KLocalizedString &trMessage, const QVariant &param)
{
    // QML `real` 等类型需与 resolveMessage 一致走 subsVariant，否则仅 toInt() 可能丢失类型信息。
    subsVariant(trMessage, param);
}

QString KLocalizedContext::i18n(const QString &message,
                                const QVariant &param1,
                                const QVariant &param2,
                                const QVariant &param3,
                                const QVariant &param4,
                                const QVariant &param5,
                                const QVariant &param6,
                                const QVariant &param7,
                                const QVariant &param8,
                                const QVariant &param9,
                                const QVariant &param10) const
{
    if (message.isEmpty()) {
        qWarning() << "i18n() needs at least one parameter";
        return QString();
    }

    KLocalizedString trMessage;
    if (!s_translationDomain.isEmpty()) {
        trMessage = ki18nd(s_translationDomain.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = ki18n(message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18nc(const QString &context,
                                 const QString &message,
                                 const QVariant &param1,
                                 const QVariant &param2,
                                 const QVariant &param3,
                                 const QVariant &param4,
                                 const QVariant &param5,
                                 const QVariant &param6,
                                 const QVariant &param7,
                                 const QVariant &param8,
                                 const QVariant &param9,
                                 const QVariant &param10) const
{
    if (context.isEmpty() || message.isEmpty()) {
        qWarning() << "i18nc() needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!s_translationDomain.isEmpty()) {
        trMessage = ki18ndc(s_translationDomain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = ki18nc(context.toUtf8().constData(), message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18np(const QString &singular,
                                 const QString &plural,
                                 const QVariant &param1,
                                 const QVariant &param2,
                                 const QVariant &param3,
                                 const QVariant &param4,
                                 const QVariant &param5,
                                 const QVariant &param6,
                                 const QVariant &param7,
                                 const QVariant &param8,
                                 const QVariant &param9,
                                 const QVariant &param10) const
{
    if (singular.isEmpty() || plural.isEmpty()) {
        qWarning() << "i18np() needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!s_translationDomain.isEmpty()) {
        trMessage = ki18ndp(s_translationDomain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = ki18np(singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18ncp(const QString &context,
                                  const QString &singular,
                                  const QString &plural,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qWarning() << "i18ncp() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!s_translationDomain.isEmpty()) {
        trMessage =
            ki18ndcp(s_translationDomain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = ki18ncp(context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18nd(const QString &domain,
                                 const QString &message,
                                 const QVariant &param1,
                                 const QVariant &param2,
                                 const QVariant &param3,
                                 const QVariant &param4,
                                 const QVariant &param5,
                                 const QVariant &param6,
                                 const QVariant &param7,
                                 const QVariant &param8,
                                 const QVariant &param9,
                                 const QVariant &param10) const
{
    if (domain.isEmpty() || message.isEmpty()) {
        qWarning() << "i18nd() needs at least two parameters";
        return QString();
    }

    KLocalizedString trMessage = ki18nd(domain.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18ndc(const QString &domain,
                                  const QString &context,
                                  const QString &message,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || message.isEmpty()) {
        qWarning() << "i18ndc() needs at least three parameters";
        return QString();
    }

    KLocalizedString trMessage = ki18ndc(domain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18ndp(const QString &domain,
                                  const QString &singular,
                                  const QString &plural,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (domain.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qWarning() << "i18ndp() needs at least three parameters";
        return QString();
    }

    KLocalizedString trMessage = ki18ndp(domain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::i18ndcp(const QString &domain,
                                   const QString &context,
                                   const QString &singular,
                                   const QString &plural,
                                   const QVariant &param1,
                                   const QVariant &param2,
                                   const QVariant &param3,
                                   const QVariant &param4,
                                   const QVariant &param5,
                                   const QVariant &param6,
                                   const QVariant &param7,
                                   const QVariant &param8,
                                   const QVariant &param9,
                                   const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qWarning() << "i18ndcp() needs at least four parameters";
        return QString();
    }

    KLocalizedString trMessage = ki18ndcp(domain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18n(const QString &message,
                                 const QVariant &param1,
                                 const QVariant &param2,
                                 const QVariant &param3,
                                 const QVariant &param4,
                                 const QVariant &param5,
                                 const QVariant &param6,
                                 const QVariant &param7,
                                 const QVariant &param8,
                                 const QVariant &param9,
                                 const QVariant &param10) const
{
    if (message.isEmpty()) {
        qWarning() << "xi18n() needs at least one parameter";
        return QString();
    }

    KLocalizedString trMessage;
    if (!s_translationDomain.isEmpty()) {
        trMessage = kxi18nd(s_translationDomain.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = kxi18n(message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18nc(const QString &context,
                                  const QString &message,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (context.isEmpty() || message.isEmpty()) {
        qWarning() << "xi18nc() needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!s_translationDomain.isEmpty()) {
        trMessage = kxi18ndc(s_translationDomain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = kxi18nc(context.toUtf8().constData(), message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18np(const QString &singular,
                                  const QString &plural,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (singular.isEmpty() || plural.isEmpty()) {
        qWarning() << "xi18np() needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!s_translationDomain.isEmpty()) {
        trMessage = kxi18ndp(s_translationDomain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = kxi18np(singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18ncp(const QString &context,
                                   const QString &singular,
                                   const QString &plural,
                                   const QVariant &param1,
                                   const QVariant &param2,
                                   const QVariant &param3,
                                   const QVariant &param4,
                                   const QVariant &param5,
                                   const QVariant &param6,
                                   const QVariant &param7,
                                   const QVariant &param8,
                                   const QVariant &param9,
                                   const QVariant &param10) const
{
    if (context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qWarning() << "xi18ncp() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!s_translationDomain.isEmpty()) {
        trMessage = kxi18ndcp(s_translationDomain.toUtf8().constData(),
                              context.toUtf8().constData(),
                              singular.toUtf8().constData(),
                              plural.toUtf8().constData());
    } else {
        trMessage = kxi18ncp(context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18nd(const QString &domain,
                                  const QString &message,
                                  const QVariant &param1,
                                  const QVariant &param2,
                                  const QVariant &param3,
                                  const QVariant &param4,
                                  const QVariant &param5,
                                  const QVariant &param6,
                                  const QVariant &param7,
                                  const QVariant &param8,
                                  const QVariant &param9,
                                  const QVariant &param10) const
{
    if (domain.isEmpty() || message.isEmpty()) {
        qWarning() << "xi18nd() needs at least two parameters";
        return QString();
    }

    KLocalizedString trMessage = kxi18nd(domain.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18ndc(const QString &domain,
                                   const QString &context,
                                   const QString &message,
                                   const QVariant &param1,
                                   const QVariant &param2,
                                   const QVariant &param3,
                                   const QVariant &param4,
                                   const QVariant &param5,
                                   const QVariant &param6,
                                   const QVariant &param7,
                                   const QVariant &param8,
                                   const QVariant &param9,
                                   const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || message.isEmpty()) {
        qWarning() << "xi18ndc() needs at least three parameters";
        return QString();
    }

    KLocalizedString trMessage =
        kxi18ndc(domain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18ndp(const QString &domain,
                                   const QString &singular,
                                   const QString &plural,
                                   const QVariant &param1,
                                   const QVariant &param2,
                                   const QVariant &param3,
                                   const QVariant &param4,
                                   const QVariant &param5,
                                   const QVariant &param6,
                                   const QVariant &param7,
                                   const QVariant &param8,
                                   const QVariant &param9,
                                   const QVariant &param10) const
{
    if (domain.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qWarning() << "xi18ndp() needs at least three parameters";
        return QString();
    }

    KLocalizedString trMessage = kxi18ndp(domain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

QString KLocalizedContext::xi18ndcp(const QString &domain,
                                    const QString &context,
                                    const QString &singular,
                                    const QString &plural,
                                    const QVariant &param1,
                                    const QVariant &param2,
                                    const QVariant &param3,
                                    const QVariant &param4,
                                    const QVariant &param5,
                                    const QVariant &param6,
                                    const QVariant &param7,
                                    const QVariant &param8,
                                    const QVariant &param9,
                                    const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qWarning() << "xi18ndcp() needs at least four parameters";
        return QString();
    }

    KLocalizedString trMessage = kxi18ndcp(domain.toUtf8().constData(),
                                           context.toUtf8().constData(),
                                           singular.toUtf8().constData(),
                                           plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    return trMessage.toString();
}

