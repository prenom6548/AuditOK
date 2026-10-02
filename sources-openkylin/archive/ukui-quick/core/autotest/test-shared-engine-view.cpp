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
#include <QGuiApplication>
#include <QPointer>
#include <QQuickItem>
#include <QResizeEvent>
#include <QTemporaryDir>
#include <QtTest>

#include "shared-engine-view.h"
#include "shared-qml-engine.h"

using namespace UkuiQuick;

namespace {

class TestableSharedEngineView : public SharedEngineView
{
public:
    using SharedEngineView::SharedEngineView;

    void sendResizeEvent(const QSize &newSize)
    {
        const QSize oldSize = size();
        resize(newSize);
        QResizeEvent event(newSize, oldSize);
        resizeEvent(&event);
    }
};

QUrl writeQml(QTemporaryDir &dir, const QString &name, const QByteArray &content)
{
    const QString path = dir.filePath(name);
    QFile file(path);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    file.write(content);
    file.close();
    return QUrl::fromLocalFile(path);
}

}

class shared_engine_view_test : public QObject
{
    Q_OBJECT

private slots:
    void sharedEngineUsesGlobalInstance();
    void constructorCreatesContextWithDefaultResizeMode();
    void emptySourceIsIgnored();
    void validSourceCreatesRootItemAndFollowsViewSize();
    void viewToRootResizeModeFollowsRootItemSize();
    void replacingSourceDeletesPreviousRootAndEmptySourceClearsIt();
    void invalidSourceKeepsRootObjectEmpty();
    void nonItemRootObjectIsRejected();
};

void shared_engine_view_test::sharedEngineUsesGlobalInstance()
{
    QCOMPARE(SharedEngineView::sharedEngine(), SharedQmlEngine::sharedQmlEngine());
    QCOMPARE(SharedEngineView::sharedEngine(), SharedEngineView::sharedEngine());
}

void shared_engine_view_test::constructorCreatesContextWithDefaultResizeMode()
{
    SharedEngineView view;

    QVERIFY(view.engine() != nullptr);
    QCOMPARE(view.engine(), SharedQmlEngine::sharedQmlEngine());
    QVERIFY(view.rootContext() != nullptr);
    QCOMPARE(view.rootContext()->parent(), &view);
    QCOMPARE(view.rootObject(), nullptr);
    QCOMPARE(view.resizeMode(), SharedEngineView::SizeRootObjectToView);

    view.setResizeMode(SharedEngineView::SizeRootObjectToView);
    QCOMPARE(view.resizeMode(), SharedEngineView::SizeRootObjectToView);
}

void shared_engine_view_test::emptySourceIsIgnored()
{
    SharedEngineView view;

    view.setSource(QUrl());

    QCOMPARE(view.rootObject(), nullptr);
}

void shared_engine_view_test::validSourceCreatesRootItemAndFollowsViewSize()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QUrl source = writeQml(dir, QStringLiteral("item.qml"), R"(
        import QtQuick 2.15
        Item {
            width: 80
            height: 30
        }
    )");

    TestableSharedEngineView view;
    view.sendResizeEvent(QSize(120, 45));

    view.setSource(source);

    QQuickItem *root = view.rootObject();
    QVERIFY(root != nullptr);
    QCOMPARE(root->parentItem(), view.contentItem());
    QCOMPARE(root->parent(), view.contentItem());
    QCOMPARE(root->width(), 120.0);
    QCOMPARE(root->height(), 45.0);

    view.sendResizeEvent(QSize(150, 60));
    QCOMPARE(root->width(), 150.0);
    QCOMPARE(root->height(), 60.0);
}

void shared_engine_view_test::viewToRootResizeModeFollowsRootItemSize()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QUrl source = writeQml(dir, QStringLiteral("sized-item.qml"), R"(
        import QtQuick 2.15
        Item {
            width: 80
            height: 30
        }
    )");

    SharedEngineView view;
    view.setResizeMode(SharedEngineView::SizeViewToRootObject);
    QCOMPARE(view.resizeMode(), SharedEngineView::SizeViewToRootObject);

    view.setSource(source);

    QQuickItem *root = view.rootObject();
    QVERIFY(root != nullptr);
    QTRY_COMPARE(view.size(), QSize(80, 30));

    root->setWidth(95);
    QTRY_COMPARE(view.size(), QSize(95, 30));

    root->setHeight(40);
    QTRY_COMPARE(view.size(), QSize(95, 40));

    root->setWidth(0);
    root->setHeight(0);
    QTRY_COMPARE(view.size(), QSize(1, 1));

    view.setResizeMode(SharedEngineView::SizeRootObjectToView);
    QCOMPARE(view.resizeMode(), SharedEngineView::SizeRootObjectToView);
    QCOMPARE(root->width(), 1.0);
    QCOMPARE(root->height(), 1.0);
}

void shared_engine_view_test::replacingSourceDeletesPreviousRootAndEmptySourceClearsIt()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QUrl firstSource = writeQml(dir, QStringLiteral("first.qml"), R"(
        import QtQuick 2.15
        Item { objectName: "first" }
    )");
    const QUrl secondSource = writeQml(dir, QStringLiteral("second.qml"), R"(
        import QtQuick 2.15
        Item { objectName: "second" }
    )");

    SharedEngineView view;

    view.setSource(firstSource);
    QPointer<QQuickItem> firstRoot = view.rootObject();
    QVERIFY(firstRoot != nullptr);
    QCOMPARE(firstRoot->objectName(), QStringLiteral("first"));

    view.setSource(secondSource);
    QQuickItem *secondRoot = view.rootObject();
    QVERIFY(secondRoot != nullptr);
    QVERIFY(firstRoot.isNull());
    QCOMPARE(secondRoot->objectName(), QStringLiteral("second"));

    view.setSource(QUrl());
    QCOMPARE(view.rootObject(), nullptr);
}

void shared_engine_view_test::invalidSourceKeepsRootObjectEmpty()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QUrl source = writeQml(dir, QStringLiteral("broken.qml"), R"(
        import QtQuick 2.15
        Item {
            property int broken:
        }
    )");

    SharedEngineView view;

    view.setSource(source);

    QCOMPARE(view.rootObject(), nullptr);
}

void shared_engine_view_test::nonItemRootObjectIsRejected()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QUrl source = writeQml(dir, QStringLiteral("object.qml"), R"(
        import QtQml 2.15
        QtObject {
            objectName: "not-an-item"
        }
    )");

    SharedEngineView view;

    view.setSource(source);

    QCOMPARE(view.rootObject(), nullptr);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    shared_engine_view_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-shared-engine-view.moc"
