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
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "widget-container.h"
#include "widget-container-item.h"
#include "widget-item-attached.h"
#include "widget-item-context.h"
#include "widget-item.h"
#include "widget.h"

using namespace UkuiQuick;

namespace {

WidgetMetadata createMetadata(const QString &basePath,
                              const QString &id,
                              bool isContainer = false,
                              const QString &mainQml = {})
{
    const QString root = QDir(basePath).filePath(id);
    QDir().mkpath(root);

    QJsonObject object{
        {QStringLiteral("Id"), id},
        {QStringLiteral("Name"), id},
        {QStringLiteral("ShowIn"), QStringLiteral("All")}
    };
    if (isContainer) {
        object.insert(QStringLiteral("WidgetType"), QStringLiteral("Container"));
    }
    if (!mainQml.isEmpty()) {
        object.insert(QStringLiteral("Contents"), QJsonObject{
            {QStringLiteral("Main"), QStringLiteral("main.qml")}
        });

        QFile qmlFile(QDir(root).filePath(QStringLiteral("main.qml")));
        qmlFile.open(QFile::WriteOnly | QFile::Truncate | QFile::Text);
        qmlFile.write(mainQml.toUtf8());
        qmlFile.close();
    }

    QFile file(QDir(root).filePath(QStringLiteral("metadata.json")));
    file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text);
    file.write(QJsonDocument(object).toJson());
    file.close();

    return WidgetMetadata(root);
}

class ExposedWidgetQuickItem : public WidgetQuickItem
{
public:
    using WidgetQuickItem::classBegin;
};

void releaseLoadedItem(Widget *widget)
{
    QMetaObject::invokeMethod(widget, "aboutToDeleted", Qt::DirectConnection);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

} // namespace

class widget_item_test : public QObject
{
    Q_OBJECT

private slots:
    void adjustingLayoutEmitsOnlyOnChange();
    void contextExposesWidgetAndContainerProperties();
    void classBeginAndAttachedPropertiesResolveWidgetContext();
    void qmlAttachedWidgetCanRequestHostFocus();
    void loadWidgetItemCreatesAndCachesValidRootItems();
    void loadWidgetItemRejectsUnexpectedRootItem();
};

void widget_item_test::adjustingLayoutEmitsOnlyOnChange()
{
    WidgetQuickItem item;
    QSignalSpy adjustingSpy(&item, &WidgetQuickItem::adjustingLayoutChanged);

    QVERIFY(!item.adjustingLayout());
    QCOMPARE(item.widgetWindow(), nullptr);

    item.setAdjustingLayout(true);
    item.setAdjustingLayout(true);

    QVERIFY(item.adjustingLayout());
    QCOMPARE(adjustingSpy.count(), 1);

    item.setAdjustingLayout(false);
    QVERIFY(!item.adjustingLayout());
    QCOMPARE(adjustingSpy.count(), 2);
}

void widget_item_test::contextExposesWidgetAndContainerProperties()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QQmlEngine engine;
    Widget widget(createMetadata(dir.path(), QStringLiteral("plain-widget")));
    WidgetContainer container(createMetadata(dir.path(), QStringLiteral("container-widget"), true));

    WidgetItemContext widgetContext(&engine, &widget);
    QCOMPARE(widgetContext.widget(), &widget);
    QCOMPARE(widgetContext.contextProperty(QStringLiteral("widget")).value<Widget *>(), &widget);
    QVERIFY(!widgetContext.contextProperty(QStringLiteral("container")).isValid());

    WidgetItemContext containerContext(&engine, &container);
    QCOMPARE(containerContext.widget(), &container);
    QCOMPARE(containerContext.contextProperty(QStringLiteral("widget")).value<Widget *>(), &container);
    QCOMPARE(containerContext.contextProperty(QStringLiteral("container")).value<WidgetContainer *>(), &container);
}

void widget_item_test::classBeginAndAttachedPropertiesResolveWidgetContext()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QQmlEngine engine;
    Widget widget(createMetadata(dir.path(), QStringLiteral("context-widget")));
    WidgetItemContext widgetContext(&engine, &widget);
    QQmlContext childContext(&widgetContext);
    ExposedWidgetQuickItem item;
    QObject childObject;

    QQmlEngine::setContextForObject(&item, &childContext);
    QQmlEngine::setContextForObject(&childObject, &childContext);

    item.classBegin();

    QCOMPARE(item.widget(), &widget);
    QCOMPARE(WidgetItemAttached::qmlAttachedProperties(&childObject), &widget);
}

void widget_item_test::qmlAttachedWidgetCanRequestHostFocus()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(createMetadata(dir.path(), QStringLiteral("focus-request-widget"), false, QStringLiteral(R"(
import QtQuick 2.15
import org.ukui.quick.widgets 1.0
WidgetItem {
    function requestFocusFromQml() {
        Widget.requestHostFocus()
    }
}
)")));
    QSignalSpy requestSpy(&widget, &Widget::hostFocusRequested);

    QPointer<WidgetQuickItem> item = WidgetQuickItem::loadWidgetItem(&widget);
    QVERIFY(item);
    QVERIFY(QMetaObject::invokeMethod(item.data(), "requestFocusFromQml", Qt::DirectConnection));

    QCOMPARE(requestSpy.count(), 1);
    QCOMPARE(qvariant_cast<Widget *>(requestSpy.first().at(0)), &widget);

    releaseLoadedItem(&widget);
    QVERIFY(item.isNull());
}

void widget_item_test::loadWidgetItemCreatesAndCachesValidRootItems()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(createMetadata(dir.path(), QStringLiteral("load-widget"), false, QStringLiteral(R"(
import QtQuick 2.15
import org.ukui.quick.widgets 1.0
WidgetItem {
    width: 10
    height: 20
}
)")));
    WidgetContainer container(createMetadata(dir.path(), QStringLiteral("load-container"), true, QStringLiteral(R"(
import QtQuick 2.15
import org.ukui.quick.container 1.0
WidgetContainerItem {
    width: 10
    height: 20
}
)")));

    QPointer<WidgetQuickItem> item = WidgetQuickItem::loadWidgetItem(&widget);
    QVERIFY(item);
    QCOMPARE(WidgetQuickItem::loadWidgetItem(&widget), item.data());
    auto *widgetItem = qobject_cast<WidgetItem *>(item.data());
    QVERIFY(widgetItem != nullptr);
    QCOMPARE(item->property("widget").value<Widget *>(), &widget);
    QCOMPARE(widget.property("widgetItem").value<WidgetItem *>(), widgetItem);
    QVERIFY(item->context() != nullptr);
    QCOMPARE(item->context()->contextProperty(QStringLiteral("widget")).value<Widget *>(), &widget);

    releaseLoadedItem(&widget);
    QVERIFY(item.isNull());

    QPointer<WidgetQuickItem> containerItem = WidgetQuickItem::loadWidgetItem(&container);
    QVERIFY(containerItem);
    QCOMPARE(WidgetQuickItem::loadWidgetItem(&container), containerItem.data());
    auto *quickContainerItem = qobject_cast<WidgetContainerItem *>(containerItem.data());
    QVERIFY(quickContainerItem != nullptr);
    QCOMPARE(containerItem->property("container").value<Widget *>(), &container);
    QCOMPARE(container.property("containerItem").value<WidgetContainerItem *>(), quickContainerItem);

    releaseLoadedItem(&container);
    QVERIFY(containerItem.isNull());
}

void widget_item_test::loadWidgetItemRejectsUnexpectedRootItem()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(createMetadata(dir.path(), QStringLiteral("invalid-root-widget"), false, QStringLiteral(R"(
import QtQuick 2.15
Item {
}
)")));

    QCOMPARE(WidgetQuickItem::loadWidgetItem(&widget), nullptr);
    QVERIFY(widget.hasUiError());
    QVERIFY(widget.uiError().contains(QStringLiteral("WidgetItem")));
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QApplication app(argc, argv);
    widget_item_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-widget-item.moc"
