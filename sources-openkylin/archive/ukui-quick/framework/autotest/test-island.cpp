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
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "island.h"

using namespace UkuiQuick;

namespace {

struct HomeGuard
{
    QByteArray oldHome = qgetenv("HOME");

    explicit HomeGuard(const QString &home)
    {
        qputenv("HOME", QFile::encodeName(home));
    }

    ~HomeGuard()
    {
        qputenv("HOME", oldHome);
    }
};

QString createWidgetPackage(const QString &basePath, const QString &id, bool isContainer)
{
    const QString root = QDir(basePath).filePath(id);
    QDir().mkpath(root);

    QJsonObject object{
        {QStringLiteral("Id"), id},
        {QStringLiteral("Name"), id},
        {QStringLiteral("ShowIn"), QStringLiteral("All")},
        {QStringLiteral("Contents"), QJsonObject{
            {QStringLiteral("Main"), QStringLiteral("main.qml")}
        }}
    };

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

class island_test : public QObject
{
    Q_OBJECT

private slots:
    void constructorInitializesConfigAndStaticNames();
    void loadMainViewCreatesDefaultViewConfig();
    void loadMainViewRejectsNonContainerWidget();
    void setMainViewReplacesExistingView();
};

void island_test::constructorInitializesConfigAndStaticNames()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    HomeGuard guard(dir.path());

    Island island(QStringLiteral("island-a"), QStringLiteral("org.demo"));

    QCOMPARE(island.id(), QStringLiteral("island-a"));
    QCOMPARE(island.app(), QStringLiteral("org.demo"));
    QVERIFY(island.config() != nullptr);
    QCOMPARE(Island::viewGroupName(), QStringLiteral("_views"));
    QCOMPARE(Island::viewGroupKey(), QStringLiteral("instanceId"));
    QCOMPARE(island.viewCount(), 0);
    QVERIFY(island.views().isEmpty());
}

void island_test::loadMainViewCreatesDefaultViewConfig()
{
    QTemporaryDir homeDir;
    QTemporaryDir widgetDir;
    QVERIFY(homeDir.isValid());
    QVERIFY(widgetDir.isValid());
    HomeGuard guard(homeDir.path());
    const QString viewId = QStringLiteral("codex-main-view");
    createWidgetPackage(widgetDir.path(), viewId, true);
    WidgetContainer::widgetLoader().setShowInFilter(WidgetMetadata::All);
    WidgetContainer::widgetLoader().addWidgetSearchPath(widgetDir.path());

    Island island(QStringLiteral("island-b"), QStringLiteral("org.demo"));
    QSignalSpy viewSpy(&island, &Island::mainViewChanged);

    QVERIFY(island.loadMainView(viewId));
    QVERIFY(island.mainView() != nullptr);
    QCOMPARE(island.mainView()->id(), viewId);
    QCOMPARE(island.mainView()->instanceId(), 0);
    QCOMPARE(island.viewCount(), 1);
    QCOMPARE(viewSpy.count(), 1);
    QCOMPARE(island.views(), QStringList({viewId}));
    QVERIFY(island.config()->child(Island::viewGroupName(), 0) != nullptr);
}

void island_test::loadMainViewRejectsNonContainerWidget()
{
    QTemporaryDir homeDir;
    QTemporaryDir widgetDir;
    QVERIFY(homeDir.isValid());
    QVERIFY(widgetDir.isValid());
    HomeGuard guard(homeDir.path());
    const QString widgetId = QStringLiteral("codex-plain-widget");
    createWidgetPackage(widgetDir.path(), widgetId, false);
    WidgetContainer::widgetLoader().setShowInFilter(WidgetMetadata::All);
    WidgetContainer::widgetLoader().addWidgetSearchPath(widgetDir.path());

    Island island(QStringLiteral("island-c"), QStringLiteral("org.demo"));

    QVERIFY(!island.loadMainView(widgetId));
    QCOMPARE(island.mainView(), nullptr);
    QCOMPARE(island.viewCount(), 1);
}

void island_test::setMainViewReplacesExistingView()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    HomeGuard guard(dir.path());
    Island island(QStringLiteral("island-d"), QStringLiteral("org.demo"));
    auto *first = new WidgetContainer;
    auto *second = new WidgetContainer;
    QSignalSpy viewSpy(&island, &Island::mainViewChanged);

    island.setMainView(first);
    QCOMPARE(island.mainView(), first);
    QCOMPARE(viewSpy.count(), 1);

    island.setMainView(second, false);
    QCOMPARE(island.mainView(), second);
    QCOMPARE(viewSpy.count(), 1);
}

QTEST_GUILESS_MAIN(island_test)

#include "test-island.moc"
