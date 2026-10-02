// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "KLocalizedString/klocalizedtranslator.h"
#include "KLocalizedString/klocalizedstring.h"

// Qt
#include <QSet>

class KLocalizedTranslatorPrivate
{
public:
    QString translationDomain;
    QSet<QString> monitoredContexts;
};

KLocalizedTranslator::KLocalizedTranslator(QObject *parent)
    : QTranslator(parent)
    , d(new KLocalizedTranslatorPrivate)
{
}

KLocalizedTranslator::~KLocalizedTranslator() = default;

void KLocalizedTranslator::setTranslationDomain(const QString &translationDomain)
{
    d->translationDomain = translationDomain;
}

void KLocalizedTranslator::addContextToMonitor(const QString &context)
{
    d->monitoredContexts.insert(context);
}

void KLocalizedTranslator::removeContextToMonitor(const QString &context)
{
    d->monitoredContexts.remove(context);
}

QString KLocalizedTranslator::translate(const char *context,
                                        const char *sourceText,
                                        const char *disambiguation,
                                        int n) const
{
    Q_UNUSED(n);

    if (d->translationDomain.isEmpty() || !d->monitoredContexts.contains(QString::fromUtf8(context))) {
        return QTranslator::translate(context, sourceText, disambiguation, n);
    }

    if (!disambiguation || qstrlen(disambiguation) == 0) {
        return ki18nd(d->translationDomain.toUtf8().constData(), sourceText).toString();
    } else {
        return ki18ndc(d->translationDomain.toUtf8().constData(), disambiguation, sourceText).toString();
    }
}

