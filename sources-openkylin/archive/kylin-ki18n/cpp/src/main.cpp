// SPDX-FileCopyrightText: 2015 Lukáš Tinkl <ltinkl@redhat.com>
// SPDX-FileCopyrightText: 2021 Ingo Klöcker <kloecker@kde.org>
// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: LGPL-2.0-or-later

// 在 QCoreApplication 启动时加载 Qt 自带翻译（QColorDialog、QFontDialog 等）。

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include <memory>

static bool loadCatalog(const QString &catalog, const QLocale &locale)
{
    auto translator = std::make_unique<QTranslator>(QCoreApplication::instance());
    if (!translator->load(locale, catalog, QString(), QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        return false;
    }
    QCoreApplication::instance()->installTranslator(translator.release());
    return true;
}

static bool loadCatalog(const QString &catalog, const QLocale &locale, const QLocale &fallbackLocale)
{
    if (loadCatalog(catalog, locale)) {
        return true;
    }
    if (fallbackLocale != locale) {
        return loadCatalog(catalog, fallbackLocale);
    }
    return false;
}

static void loadTranslation(const QString &localeName, const QString &fallbackLocaleName)
{
    const QLocale locale{localeName};
    const QLocale fallbackLocale{fallbackLocaleName};
    if (loadCatalog(QStringLiteral("qt_"), locale, fallbackLocale)) {
        return;
    }
    const auto catalogs = {
        QStringLiteral("qtbase_"),
        QStringLiteral("qtscript_"),
        QStringLiteral("qtmultimedia_"),
        QStringLiteral("qtxmlpatterns_"),
    };
    for (const auto &catalog : catalogs) {
        loadCatalog(catalog, locale, fallbackLocale);
    }
}

static void load()
{
    loadCatalog(QStringLiteral("qt_"), QLocale{QStringLiteral("en")});

    const QLocale locale = QLocale::system();
    if (locale.name() != QStringLiteral("en")) {
        loadTranslation(locale.name(), locale.bcp47Name());
    }
}

Q_COREAPP_STARTUP_FUNCTION(load)
