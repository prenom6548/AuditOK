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

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

#include "widget-loader.h"
#include "widget-container.h"

using namespace UkuiQuick;

namespace {

QString createWidgetPackage(const QString &basePath, const QString &id, WidgetMetadata::Hosts hosts,
                            const QStringList &applicationIds = {}, bool isContainer = false)
{
    const QString root = QDir(basePath).filePath(id);
    QDir().mkpath(root);

    QJsonObject object{
        {QStringLiteral("Id"), id},
        {QStringLiteral("Name"), id},
        {QStringLiteral("ShowIn"), hosts.testFlag(WidgetMetadata::All) ? QStringLiteral("All")
                                                                       : QString(hosts.testFlag(WidgetMetadata::Panel) ? QStringLiteral("Panel") : QStringLiteral("SideBar"))},
        {QStringLiteral("Contents"), QJsonObject{
            {QStringLiteral("Main"), QStringLiteral("main.qml")}
        }}
    };

    if (!applicationIds.isEmpty()) {
        QJsonArray apps;
        for (const QString &appId : applicationIds) {
            apps.append(appId);
        }
        object.insert(QStringLiteral("ApplicationId"), apps);
    }

    if (isContainer) {
        object.insert(QStringLiteral("WidgetType"), QStringLiteral("Container"));
    }

    QFile file(QDir(root).filePath(QStringLiteral("metadata.json")));
    file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text);
    file.write(QJsonDocument(object).toJson());
    file.close();

    return root;
}

}

class widget_loader_test : public QObject
{
    Q_OBJECT

private slots:
    void metadataAndWidgetCanBeLoadedFromCustomPath();
    void widgetsAreFilteredByHostAndApplication();
    void widgetsMetadataKeepsUniqueEntries();
};

void widget_loader_test::metadataAndWidgetCanBeLoadedFromCustomPath()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    createWidgetPackage(dir.path(), QStringLiteral("plain-widget"), WidgetMetadata::All, {QStringLiteral("org.demo")});
    createWidgetPackage(dir.path(), QStringLiteral("container-widget"), WidgetMetadata::Panel, {QStringLiteral("org.demo")}, true);

    WidgetLoader loader;
    loader.addWidgetSearchPath(dir.path());

    WidgetMetadata metadata = loader.loadMetadata(QStringLiteral("plain-widget"));
    QVERIFY(metadata.isValid());
    QCOMPARE(metadata.id(), QStringLiteral("plain-widget"));

    std::unique_ptr<Widget> plain(loader.loadWidget(QStringLiteral("plain-widget")));
    QVERIFY(plain != nullptr);
    QVERIFY(qobject_cast<WidgetContainer *>(plain.get()) == nullptr);

    std::unique_ptr<Widget> container(loader.loadWidget(QStringLiteral("container-widget")));
    QVERIFY(container != nullptr);
    QVERIFY(qobject_cast<WidgetContainer *>(container.get()) != nullptr);
}

void widget_loader_test::widgetsAreFilteredByHostAndApplication()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    createWidgetPackage(dir.path(), QStringLiteral("panel-only"), WidgetMetadata::Panel, {QStringLiteral("org.demo")});
    createWidgetPackage(dir.path(), QStringLiteral("sidebar-only"), WidgetMetadata::SideBar, {QStringLiteral("org.demo")});
    createWidgetPackage(dir.path(), QStringLiteral("all-hosts"), WidgetMetadata::All, {QStringLiteral("org.demo"), QStringLiteral("org.other")});

    WidgetLoader loader;
    loader.addWidgetSearchPath(dir.path());

    QVERIFY(loader.widgets().isEmpty());

    loader.setShowInFilter(WidgetMetadata::Panel);
    const QStringList panelWidgets = loader.widgets(QStringLiteral("org.demo"));
    QCOMPARE(panelWidgets.size(), 2);
    QVERIFY(panelWidgets.contains(QStringLiteral("panel-only")));
    QVERIFY(panelWidgets.contains(QStringLiteral("all-hosts")));
    QVERIFY(!panelWidgets.contains(QStringLiteral("sidebar-only")));

    loader.setShowInFilter(WidgetMetadata::SideBar);
    std::unique_ptr<Widget> rejected(loader.loadWidget(QStringLiteral("panel-only")));
    QVERIFY(rejected == nullptr);
}

void widget_loader_test::widgetsMetadataKeepsUniqueEntries()
{
    QTemporaryDir firstDir;
    QTemporaryDir secondDir;
    QVERIFY(firstDir.isValid());
    QVERIFY(secondDir.isValid());
    const QString duplicateId = QStringLiteral("codex-duplicate-widget");
    createWidgetPackage(firstDir.path(), duplicateId, WidgetMetadata::All, {QStringLiteral("org.demo")});
    createWidgetPackage(secondDir.path(), duplicateId, WidgetMetadata::All, {QStringLiteral("org.demo")});

    WidgetLoader loader;
    loader.addWidgetSearchPath(firstDir.path());
    loader.addWidgetSearchPath(secondDir.path());

    const QList<WidgetMetadata> metadataList = loader.widgetsMetadata();
    int matches = 0;
    for (const WidgetMetadata &metadata : metadataList) {
        if (metadata.id() == duplicateId) {
            ++matches;
        }
    }
    QCOMPARE(matches, 1);
}

QTEST_GUILESS_MAIN(widget_loader_test)

#include "test-widget-loader.moc"
