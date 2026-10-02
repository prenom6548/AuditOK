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
#include <QMouseEvent>
#include <QPointer>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTemporaryDir>
#include <QtTest>

#include "widget-container-item.h"
#include "widget-container.h"
#include "widget-item-context.h"
#include "widget.h"

using namespace UkuiQuick;

namespace {

class ExposedWidgetContainerItem : public WidgetContainerItem
{
public:
    using WidgetContainerItem::classBegin;
    using WidgetContainerItem::event;
};

void setContextForContainer(QObject *object, QQmlEngine *engine, WidgetContainer *container)
{
    auto *containerContext = new WidgetItemContext(engine, container, object);
    auto *childContext = new QQmlContext(containerContext, object);
    QQmlEngine::setContextForObject(object, childContext);
}

WidgetMetadata createWidgetPackage(const QString &basePath, const QString &id, const QByteArray &mainQml)
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

} // namespace

class widget_container_item_test : public QObject
{
    Q_OBJECT

private slots:
    void modelMaintainsFlatWidgetItemList();
    void itemHandlesMissingAndContextContainer();
    void attachedPropertiesFindContainerThroughContextChain();
    void containerSignalsKeepWidgetItemModelInSync();
    void rightClickWithoutActionsDoesNotActivateContainer();
    void rightClickWithContainerActionsActivatesAndLeftClickClosesMenu();
};

void widget_container_item_test::modelMaintainsFlatWidgetItemList()
{
    WidgetItemModel model;
    WidgetItem first;
    WidgetItem second;

    QVERIFY(!model.insertWidgetItem(nullptr));
    QVERIFY(model.insertWidgetItem(&first));
    QVERIFY(!model.insertWidgetItem(&first));
    QVERIFY(model.insertWidgetItem(&second));

    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(model.columnCount(QModelIndex()), 1);
    QCOMPARE(model.roleNames().value(WidgetItemModel::WidgetItemRole), QByteArray("widgetItem"));

    const QModelIndex firstIndex = model.index(0, 0);
    QVERIFY(firstIndex.isValid());
    QCOMPARE(model.rowCount(firstIndex), 0);
    QCOMPARE(model.columnCount(firstIndex), 0);
    QCOMPARE(model.data(firstIndex, WidgetItemModel::WidgetItemRole).value<WidgetQuickItem *>(), &first);
    QCOMPARE(model.data(firstIndex, Qt::DisplayRole).value<WidgetQuickItem *>(), &first);

    QVERIFY(model.removeWidgetItem(&first));
    QCOMPARE(model.rowCount(QModelIndex()), 1);
    QCOMPARE(model.data(model.index(0, 0), WidgetItemModel::WidgetItemRole).value<WidgetQuickItem *>(), &second);
    QVERIFY(!model.removeWidgetItem(&first));
    QVERIFY(!model.removeWidgetItem(nullptr));
}

void widget_container_item_test::itemHandlesMissingAndContextContainer()
{
    ExposedWidgetContainerItem item;
    QQmlEngine engine;
    WidgetContainer container;
    ExposedWidgetContainerItem contextItem;
    setContextForContainer(&contextItem, &engine, &container);

    QVERIFY(item.widgetItemModel() != nullptr);
    QCOMPARE(item.container(), nullptr);
    QCOMPARE(item.component(), nullptr);
    QCOMPARE(item.widgetItem(123), nullptr);
    QCOMPARE(item.widgetItemForWidget(nullptr), nullptr);

    contextItem.classBegin();

    QCOMPARE(contextItem.container(), &container);
    QCOMPARE(WidgetContainerItemAttached::qmlAttachedProperties(&contextItem), &container);
}

void widget_container_item_test::attachedPropertiesFindContainerThroughContextChain()
{
    QQmlEngine engine;
    WidgetContainer container;
    QObject plainObject;
    setContextForContainer(&plainObject, &engine, &container);

    QCOMPARE(WidgetContainerItemAttached::qmlAttachedProperties(&plainObject), &container);
}

void widget_container_item_test::containerSignalsKeepWidgetItemModelInSync()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QQmlEngine engine;
    WidgetContainer container;
    ExposedWidgetContainerItem item;
    QSignalSpy addedSpy(&item, &WidgetContainerItem::widgetItemAdded);
    QSignalSpy removedSpy(&item, &WidgetContainerItem::widgetItemRemoved);
    const WidgetMetadata metadata = createWidgetPackage(dir.path(),
                                                        QStringLiteral("container-item-widget"),
                                                        R"(
import QtQuick 2.15
import org.ukui.quick.widgets 1.0

WidgetItem {
}
)");
    auto *widget = new Widget(metadata);
    QPointer<Widget> guardedWidget(widget);

    widget->setInstanceId(42);
    setContextForContainer(&item, &engine, &container);
    item.classBegin();

    container.addWidget(widget);

    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(item.widgetItemModel()->rowCount(QModelIndex()), 1);
    QCOMPARE(widget->container(), &container);

    auto *widgetItem = item.widgetItem(42);
    QVERIFY(widgetItem != nullptr);
    QCOMPARE(item.widgetItemForWidget(widget), widgetItem);
    QCOMPARE(item.widgetItemModel()->data(item.widgetItemModel()->index(0, 0),
                                          WidgetItemModel::WidgetItemRole).value<WidgetQuickItem *>(),
             static_cast<WidgetQuickItem *>(widgetItem));

    container.removeWidget(widget);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(item.widgetItemModel()->rowCount(QModelIndex()), 0);
    QCOMPARE(item.widgetItem(42), nullptr);
    QVERIFY(guardedWidget.isNull());
}

void widget_container_item_test::rightClickWithoutActionsDoesNotActivateContainer()
{
    QQmlEngine engine;
    WidgetContainer container;
    ExposedWidgetContainerItem item;
    setContextForContainer(&item, &engine, &container);
    item.classBegin();

    QMouseEvent event(QEvent::MouseButtonPress,
                      QPointF(1, 1),
                      Qt::RightButton,
                      Qt::RightButton,
                      Qt::NoModifier);

    item.event(&event);
    QVERIFY(!container.active());
}

void widget_container_item_test::rightClickWithContainerActionsActivatesAndLeftClickClosesMenu()
{
    QQmlEngine engine;
    WidgetContainer container;
    ExposedWidgetContainerItem item;
    QAction action(QStringLiteral("Open"), &container);
    container.setActions({&action});
    setContextForContainer(&item, &engine, &container);
    item.classBegin();

    QMouseEvent rightClick(QEvent::MouseButtonPress,
                           QPointF(2, 2),
                           Qt::RightButton,
                           Qt::RightButton,
                           Qt::NoModifier);
    QVERIFY(item.event(&rightClick));
    QTRY_VERIFY(container.active());

    QMouseEvent leftClick(QEvent::MouseButtonPress,
                          QPointF(2, 2),
                          Qt::LeftButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
    item.event(&leftClick);
    QTRY_VERIFY(!container.active());
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QApplication app(argc, argv);
    widget_container_item_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-widget-container-item.moc"
