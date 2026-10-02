// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "KLocalizedString/kuitsetup.h"
#include "KLocalizedString/klocalizedstring.h"
#include "kuitsetup_p.h"

#include <QByteArray>
#include <QHash>
#include <QMutex>
#include <QString>
#include <QStringList>

void ki18n_cpp_install_default_kuit_markup(KuitSetup *setup);

KuitSetup::KuitSetup(const QByteArray &domain)
    : d(new KuitSetupPrivate)
{
    d->domain = domain;
    ki18n_cpp_install_default_kuit_markup(this);
}

KuitSetup::~KuitSetup() = default;

namespace {

QMutex s_kuitSetupMutex;
QHash<QByteArray, KuitSetup *> s_kuitSetups;

} // namespace

namespace Kuit
{

KuitSetup &setupForDomain(const char *domain)
{
    const QByteArray dom = domain ? QByteArray(domain) : QByteArray();
    return setupForDomain(dom);
}

KuitSetup &setupForDomain(const QByteArray &domain)
{
    QMutexLocker locker(&s_kuitSetupMutex);
    KuitSetup *&slot = s_kuitSetups[domain];
    if (!slot) {
        slot = new KuitSetup(domain);
    }
    return *slot;
}

}
