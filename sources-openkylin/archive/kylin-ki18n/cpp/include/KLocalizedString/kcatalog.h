/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2013 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCATALOG_H
#define KCATALOG_H

#include <ki18n_export.h>

#include <QByteArray>
#include <QHash>
#include <QLocale>
#include <QSet>
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>

class KCatalogPrivate;

/**
 * @class KCatalog kcatalog.h <KCatalog>
 *
 * \short Load and manage Gettext catalogs.
 */
class KI18N_EXPORT KCatalog
{
public:
    KCatalog();
    explicit KCatalog(const QByteArray &domain);
    KCatalog(const QByteArray &domain, const QString &language);
    KCatalog(const KCatalog &);
    KCatalog &operator=(const KCatalog &);
    ~KCatalog();

    bool isNull() const;

    QString translate(const QByteArray &msgid,
                      const QByteArray &msgidPlural = QByteArray(),
                      qint64 n = -1,
                      const QByteArray &context = QByteArray()) const;

    QString translate(const QByteArray &msgid) const;
    QString translate(const QByteArray &msgctxt, const QByteArray &msgid) const;
    QString translate(const QByteArray &msgid, const QByteArray &msgid_plural, qulonglong n) const;
    QString translate(const QByteArray &msgctxt, const QByteArray &msgid, 
                      const QByteArray &msgid_plural, qulonglong n) const;

    QString localeDir() const;

    QStringList languages() const;

    static QString catalogLocaleDir(const QByteArray &domain, const QString &language);
    static void addDomainLocaleDir(const QByteArray &domain, const QString &path);
    static QSet<QString> availableCatalogLanguages(const QByteArray &domain);

private:
    QSharedDataPointer<KCatalogPrivate> d;
};

#endif // KCATALOG_H

