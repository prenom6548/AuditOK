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

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QResizeEvent>
#include <QTemporaryDir>
#include <QtTest>

#include "view/island-view.h"

using namespace UkuiQuick;

namespace {

class HomeGuard
{
public:
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

class TestableIslandView : public IslandView
{
public:
    using IslandView::IslandView;

    void sendResizeEvent(const QSize &newSize)
    {
        const QSize oldSize = size();
        resize(newSize);
        QResizeEvent event(newSize, oldSize);
        resizeEvent(&event);
    }
};

WidgetMetadata createContainerPackage(const QString &basePath,
                                      const QString &id,
                                      const QByteArray &mainQml)
{
    const QString root = QDir(basePath).filePath(id);
    QDir().mkpath(root);

    QJsonObject object{
        {QStringLiteral("Id"), id},
        {QStringLiteral("Name"), id},
        {QStringLiteral("ShowIn"), QStringLiteral("All")},
        {QStringLiteral("WidgetType"), QStringLiteral("Container")},
        {QStringLiteral("Contents"), QJsonObject{
            {QStringLiteral("Main"), QStringLiteral("main.qml")}
        }}
    };

    QFile metadataFile(QDir(root).filePath(QStringLiteral("metadata.json")));
    metadataFile.open(QFile::WriteOnly | QFile::Truncate | QFile::Text);
    metadataFile.write(QJsonDocument(object).toJson());
    metadataFile.close();

    QFile qmlFile(QDir(root).filePath(QStringLiteral("main.qml")));
    qmlFile.open(QFile::WriteOnly | QFile::Truncate | QFile::Text);
    qmlFile.write(mainQml);
    qmlFile.close();

    return WidgetMetadata(root);
}

void prepareWidgetSearchPath(const QString &path)
{
    WidgetContainer::widgetLoader().setShowInFilter(WidgetMetadata::All);
    WidgetContainer::widgetLoader().addWidgetSearchPath(path);
}

} // namespace

class island_view_test : public QObject
{
    Q_OBJECT

private slots:
    void constructorInitializesIslandAndContext();
    void loadMainViewWithoutItemDefersUiCreation();
    void viewToRootResizeModeTracksRootItemSize();
    void setMainViewLoadsProvidedContainer();
};

void island_view_test::constructorInitializesIslandAndContext()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    HomeGuard guard(dir.path());

    {
        TestableIslandView view(QStringLiteral("island-view-a"), QStringLiteral("org.demo"));

        QVERIFY(view.island() != nullptr);
        QCOMPARE(view.island()->id(), QStringLiteral("island-view-a"));
        QCOMPARE(view.island()->app(), QStringLiteral("org.demo"));
        QCOMPARE(view.config(), view.island()->config());
        QVERIFY(view.rootContext() != nullptr);
        QCOMPARE(view.rootContext()->parent(), &view);
        QCOMPARE(view.rootContext()->contextProperty(QStringLiteral("island")).value<QObject *>(),
                 static_cast<QObject *>(&view));
        QCOMPARE(view.mainView(), nullptr);
        QCOMPARE(view.rootItem(), nullptr);
        QCOMPARE(view.resizeMode(), IslandView::SizeRootObjectToView);

        view.setSource(QUrl());
        QCOMPARE(view.rootItem(), nullptr);
        QVERIFY(!view.loadMainViewItem());
    }

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

void island_view_test::loadMainViewWithoutItemDefersUiCreation()
{
    QTemporaryDir homeDir;
    QTemporaryDir widgetDir;
    QVERIFY(homeDir.isValid());
    QVERIFY(widgetDir.isValid());
    HomeGuard guard(homeDir.path());
    const QString viewId = QStringLiteral("codex-island-view-deferred");
    createContainerPackage(widgetDir.path(), viewId, R"(
import QtQuick 2.15
import org.ukui.quick.container 1.0

WidgetContainerItem {
    objectName: "deferred-root"
    width: 80
    height: 30
}
)");
    prepareWidgetSearchPath(widgetDir.path());

    {
        TestableIslandView view(QStringLiteral("island-view-b"), QStringLiteral("org.demo"));
        QSignalSpy mainViewChangedSpy(view.island(), &Island::mainViewChanged);

        view.sendResizeEvent(QSize(120, 45));

        QVERIFY(view.loadMainViewWithoutItem(viewId, 7));
        QVERIFY(view.mainView() != nullptr);
        QCOMPARE(view.mainView()->id(), viewId);
        QCOMPARE(view.mainView()->instanceId(), 7);
        QCOMPARE(view.mainView()->island(), view.island());
        QCOMPARE(view.rootItem(), nullptr);
        QCOMPARE(mainViewChangedSpy.count(), 0);

        QVERIFY(view.loadMainViewItem());
        QVERIFY(view.rootItem() != nullptr);
        QCOMPARE(view.rootItem()->objectName(), QStringLiteral("deferred-root"));
        QCOMPARE(view.rootItem()->container(), view.mainView());
        QCOMPARE(view.rootItem()->parentItem(), view.contentItem());
        QCOMPARE(view.rootItem()->context()->parentContext(), view.rootContext());
        QCOMPARE(view.rootItem()->width(), 120.0);
        QCOMPARE(view.rootItem()->height(), 45.0);
    }

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

void island_view_test::viewToRootResizeModeTracksRootItemSize()
{
    QTemporaryDir homeDir;
    QTemporaryDir widgetDir;
    QVERIFY(homeDir.isValid());
    QVERIFY(widgetDir.isValid());
    HomeGuard guard(homeDir.path());
    const QString viewId = QStringLiteral("codex-island-view-resize");
    createContainerPackage(widgetDir.path(), viewId, R"(
import QtQuick 2.15
import org.ukui.quick.container 1.0

WidgetContainerItem {
    width: 80
    height: 30
}
)");
    prepareWidgetSearchPath(widgetDir.path());

    {
        TestableIslandView view(QStringLiteral("island-view-c"), QStringLiteral("org.demo"));
        QSignalSpy mainViewChangedSpy(view.island(), &Island::mainViewChanged);

        view.setResizeMode(IslandView::SizeViewToRootObject);
        QCOMPARE(view.resizeMode(), IslandView::SizeViewToRootObject);

        QVERIFY(view.loadMainView(viewId));
        QCOMPARE(mainViewChangedSpy.count(), 1);
        QVERIFY(view.rootItem() != nullptr);
        QTRY_COMPARE(view.size(), QSize(80, 30));

        view.rootItem()->setWidth(95);
        QTRY_COMPARE(view.size(), QSize(95, 30));

        view.rootItem()->setHeight(40);
        QTRY_COMPARE(view.size(), QSize(95, 40));

        view.rootItem()->setWidth(0);
        view.rootItem()->setHeight(0);
        QTRY_COMPARE(view.size(), QSize(1, 1));

        view.setResizeMode(IslandView::SizeRootObjectToView);
        QCOMPARE(view.resizeMode(), IslandView::SizeRootObjectToView);
        QCOMPARE(view.rootItem()->width(), 1.0);
        QCOMPARE(view.rootItem()->height(), 1.0);
    }

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

void island_view_test::setMainViewLoadsProvidedContainer()
{
    QTemporaryDir homeDir;
    QTemporaryDir widgetDir;
    QVERIFY(homeDir.isValid());
    QVERIFY(widgetDir.isValid());
    HomeGuard guard(homeDir.path());
    const QString viewId = QStringLiteral("codex-island-view-manual");
    const WidgetMetadata metadata = createContainerPackage(widgetDir.path(), viewId, R"(
import QtQuick 2.15
import org.ukui.quick.container 1.0

WidgetContainerItem {
    objectName: "manual-root"
    width: 20
    height: 10
}
)");

    {
        TestableIslandView view(QStringLiteral("island-view-d"), QStringLiteral("org.demo"));
        QSignalSpy mainViewChangedSpy(view.island(), &Island::mainViewChanged);
        auto *container = new WidgetContainer(metadata);
        container->setInstanceId(3);

        view.sendResizeEvent(QSize(64, 32));
        view.setMainView(container);

        QCOMPARE(mainViewChangedSpy.count(), 1);
        QCOMPARE(view.mainView(), container);
        QCOMPARE(view.mainView()->instanceId(), 3);
        QCOMPARE(view.mainView()->island(), view.island());
        QVERIFY(view.rootItem() != nullptr);
        QCOMPARE(view.rootItem()->objectName(), QStringLiteral("manual-root"));
        QCOMPARE(view.rootItem()->container(), container);
        QCOMPARE(view.rootItem()->width(), 64.0);
        QCOMPARE(view.rootItem()->height(), 32.0);
    }

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QApplication app(argc, argv);
    island_view_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-island-view.moc"
