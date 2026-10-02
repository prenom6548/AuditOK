/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QTemporaryDir>
#include <QtTest>

#include "widget-metadata.h"

using namespace UkuiQuick;

namespace {

bool writeMetadata(const QString &root, const QJsonObject &object)
{
    QDir().mkpath(root);
    QFile file(QDir(root).absoluteFilePath(QStringLiteral("metadata.json")));
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        return false;
    }

    file.write(QJsonDocument(object).toJson());
    return true;
}

}

class widget_metadata_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultMetadataIsInvalidAndUsesFallbacks();
    void validMetadataReadsAllSupportedFields();
    void invalidPackagesAreRejected();
    void copyAndAssignmentShareMetadataIdentity();
};

void widget_metadata_test::defaultMetadataIsInvalidAndUsesFallbacks()
{
    WidgetMetadata metadata;

    QVERIFY(!metadata.isValid());
    QCOMPARE(metadata.id(), QString());
    QCOMPARE(metadata.icon(), QStringLiteral("application-x-desktop"));
    QCOMPARE(metadata.name(), QString());
    QCOMPARE(metadata.tooltip(), QString());
    QCOMPARE(metadata.version(), QString());
    QCOMPARE(metadata.website(), QString());
    QCOMPARE(metadata.bugReport(), QStringLiteral("https://gitee.com/openkylin/ukui-panel/issues"));
    QCOMPARE(metadata.description(), QString());
    QVERIFY(metadata.authors().isEmpty());
    QVERIFY(metadata.contents().isEmpty());
    QCOMPARE(metadata.showIn(), WidgetMetadata::All);
    QCOMPARE(metadata.widgetType(), Types::Widget);
    QVERIFY(metadata.applicationId().isEmpty());
    QCOMPARE(metadata.thumbnail(), QString());
}

void widget_metadata_test::validMetadataReadsAllSupportedFields()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString root = dir.filePath(QStringLiteral("demo-widget"));
    const QString localeNameKey = QStringLiteral("Name[%1]").arg(QLocale::system().name());
    const QString localeTooltipKey = QStringLiteral("Tooltip[%1]").arg(QLocale::system().name());
    const QString localeDescriptionKey = QStringLiteral("Description[%1]").arg(QLocale::system().name());
    QVERIFY(writeMetadata(root, {
        {QStringLiteral("Id"), QStringLiteral("demo-widget")},
        {QStringLiteral("Icon"), QStringLiteral("demo-icon")},
        {QStringLiteral("Name"), QStringLiteral("Demo")},
        {localeNameKey, QStringLiteral("Localized Demo")},
        {QStringLiteral("Tooltip"), QStringLiteral("Demo tooltip")},
        {localeTooltipKey, QStringLiteral("Localized tooltip")},
        {QStringLiteral("Version"), QStringLiteral("1.2.3")},
        {QStringLiteral("Website"), QStringLiteral("https://example.test")},
        {QStringLiteral("BugReport"), QStringLiteral("https://example.test/bugs")},
        {QStringLiteral("Description"), QStringLiteral("Demo description")},
        {localeDescriptionKey, QStringLiteral("Localized description")},
        {QStringLiteral("Authors"), QJsonArray { QJsonObject {{ QStringLiteral("Name"), QStringLiteral("Alice") }} }},
        {QStringLiteral("Contents"), QJsonObject {{ QStringLiteral("Main"), QStringLiteral("main.qml") }}},
        {QStringLiteral("ShowIn"), QStringLiteral("Panel,Desktop,TaskManager,Wallpaper,DesktopView,Search")},
        {QStringLiteral("WidgetType"), QStringLiteral("Container")},
        {QStringLiteral("ApplicationId"), QJsonArray { QStringLiteral("org.ukui.demo"), QStringLiteral("org.ukui.second") }},
        {QStringLiteral("Thumbnail"), QStringLiteral("thumbnail.png")}
    }));

    WidgetMetadata metadata(root);

    QVERIFY(metadata.isValid());
    QCOMPARE(metadata.root().absolutePath(), QDir(root).absolutePath());
    QCOMPARE(metadata.id(), QStringLiteral("demo-widget"));
    QCOMPARE(metadata.icon(), QStringLiteral("demo-icon"));
    QCOMPARE(metadata.name(), QStringLiteral("Localized Demo"));
    QCOMPARE(metadata.tooltip(), QStringLiteral("Localized tooltip"));
    QCOMPARE(metadata.version(), QStringLiteral("1.2.3"));
    QCOMPARE(metadata.website(), QStringLiteral("https://example.test"));
    QCOMPARE(metadata.bugReport(), QStringLiteral("https://example.test/bugs"));
    QCOMPARE(metadata.description(), QStringLiteral("Localized description"));
    QCOMPARE(metadata.authors().size(), 1);
    QCOMPARE(metadata.contents().value(QStringLiteral("Main")).toString(), QStringLiteral("main.qml"));
    QVERIFY(metadata.showIn().testFlag(WidgetMetadata::Panel));
    QVERIFY(metadata.showIn().testFlag(WidgetMetadata::Desktop));
    QVERIFY(metadata.showIn().testFlag(WidgetMetadata::TaskManager));
    QVERIFY(metadata.showIn().testFlag(WidgetMetadata::Wallpaper));
    QVERIFY(metadata.showIn().testFlag(WidgetMetadata::DesktopView));
    QVERIFY(metadata.showIn().testFlag(WidgetMetadata::Search));
    QVERIFY(!metadata.showIn().testFlag(WidgetMetadata::SideBar));
    QCOMPARE(metadata.widgetType(), Types::Container);
    QCOMPARE(metadata.applicationId(), QStringList({QStringLiteral("org.ukui.demo"), QStringLiteral("org.ukui.second")}));
    QCOMPARE(metadata.thumbnail(), QStringLiteral("thumbnail.png"));
}

void widget_metadata_test::invalidPackagesAreRejected()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString missingMetadataRoot = dir.filePath(QStringLiteral("missing"));
    QDir().mkpath(missingMetadataRoot);
    WidgetMetadata missingMetadata(missingMetadataRoot);
    QVERIFY(!missingMetadata.isValid());

    const QString idMismatchRoot = dir.filePath(QStringLiteral("actual-id"));
    QVERIFY(writeMetadata(idMismatchRoot, {
        {QStringLiteral("Id"), QStringLiteral("different-id")}
    }));
    WidgetMetadata idMismatch(idMismatchRoot);
    QVERIFY(!idMismatch.isValid());

    const QString invalidJsonRoot = dir.filePath(QStringLiteral("invalid-json"));
    QDir().mkpath(invalidJsonRoot);
    QFile file(QDir(invalidJsonRoot).absoluteFilePath(QStringLiteral("metadata.json")));
    QVERIFY(file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text));
    file.write("{");
    file.close();
    WidgetMetadata invalidJson(invalidJsonRoot);
    QVERIFY(!invalidJson.isValid());
}

void widget_metadata_test::copyAndAssignmentShareMetadataIdentity()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString root = dir.filePath(QStringLiteral("copy-widget"));
    QVERIFY(writeMetadata(root, {
        {QStringLiteral("Id"), QStringLiteral("copy-widget")},
        {QStringLiteral("Name"), QStringLiteral("Copy Widget")}
    }));

    WidgetMetadata original(root);
    WidgetMetadata copied(original);
    WidgetMetadata assigned;
    assigned = original;

    QVERIFY(original == copied);
    QVERIFY(original == assigned);
    QCOMPARE(copied.name(), QStringLiteral("Copy Widget"));
    QCOMPARE(assigned.id(), QStringLiteral("copy-widget"));
}

QTEST_GUILESS_MAIN(widget_metadata_test)

#include "test-widget-metadata.moc"
