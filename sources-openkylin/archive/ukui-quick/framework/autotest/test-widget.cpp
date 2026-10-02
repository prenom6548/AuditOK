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

#include <QAction>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "widget.h"
#include "widget-container.h"

using namespace UkuiQuick;

namespace {

QString createWidgetPackage(const QString &basePath, const QString &id, const QJsonObject &contents = {})
{
    const QString root = QDir(basePath).filePath(id);
    QDir().mkpath(root);

    QFile file(QDir(root).filePath(QStringLiteral("metadata.json")));
    file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text);
    file.write(QJsonDocument(QJsonObject{
        {QStringLiteral("Id"), id},
        {QStringLiteral("Icon"), QStringLiteral("demo-icon")},
        {QStringLiteral("Name"), QStringLiteral("Demo Name")},
        {QStringLiteral("Tooltip"), QStringLiteral("Demo Tooltip")},
        {QStringLiteral("Version"), QStringLiteral("1.0")},
        {QStringLiteral("ShowIn"), QStringLiteral("Panel")},
        {QStringLiteral("Contents"), contents}
    }).toJson());
    file.close();

    return root;
}

WidgetMetadata loadMetadata(const QString &basePath, const QString &id, const QJsonObject &contents = {})
{
    return WidgetMetadata(createWidgetPackage(basePath, id, contents));
}

}

class widget_test : public QObject
{
    Q_OBJECT

private slots:
    void propertiesAndActionsReflectMetadataAndMutations();
    void configMapsAreCreatedFromAssignedConfig();
    void containerChangesPropagateOrientation();
    void visibleReflectsAssignedState();
    void requestHostFocusEmitsForEveryCall();
};

void widget_test::propertiesAndActionsReflectMetadataAndMutations()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(loadMetadata(dir.path(), QStringLiteral("demo-widget")));
    QAction first(QStringLiteral("first"), &widget);
    QAction second(QStringLiteral("second"), &widget);
    QAction extra(QStringLiteral("extra"), &widget);
    QSignalSpy iconSpy(&widget, &Widget::iconChanged);
    QSignalSpy nameSpy(&widget, &Widget::nameChanged);
    QSignalSpy tooltipSpy(&widget, &Widget::tooltipChanged);
    QSignalSpy versionSpy(&widget, &Widget::versionChanged);
    QSignalSpy activeSpy(&widget, &Widget::activeChanged);
    QSignalSpy ratioSpy(&widget, &Widget::ratioChanged);

    QCOMPARE(widget.id(), QStringLiteral("demo-widget"));
    QCOMPARE(widget.icon(), QStringLiteral("demo-icon"));
    QCOMPARE(widget.name(), QStringLiteral("Demo Name"));
    QCOMPARE(widget.tooltip(), QStringLiteral("Demo Tooltip"));
    QCOMPARE(widget.version(), QStringLiteral("1.0"));
    QVERIFY(widget.showIn().testFlag(WidgetMetadata::Panel));
    QVERIFY(!widget.showIn().testFlag(WidgetMetadata::SideBar));

    widget.setIcon(QStringLiteral("new-icon"));
    widget.setName(QStringLiteral("New Name"));
    widget.setTooltip(QStringLiteral("New Tooltip"));
    widget.setVersion(QStringLiteral("2.0"));
    widget.setActions({&first, &second});
    widget.setAppendActions({&extra});
    widget.setUiError(QStringLiteral("broken"));
    widget.setActive(true);
    widget.setActive(true);
    widget.setRatio(1.5);
    widget.setRatio(1.5);

    QCOMPARE(widget.icon(), QStringLiteral("new-icon"));
    QCOMPARE(widget.name(), QStringLiteral("New Name"));
    QCOMPARE(widget.tooltip(), QStringLiteral("New Tooltip"));
    QCOMPARE(widget.version(), QStringLiteral("2.0"));
    QCOMPARE(widget.actions().size(), 3);
    QCOMPARE(widget.actions().at(0), &first);
    QCOMPARE(widget.actions().at(2), &extra);
    QVERIFY(widget.hasUiError());
    QCOMPARE(widget.uiError(), QStringLiteral("broken"));
    QVERIFY(widget.active());
    QCOMPARE(widget.ratio(), 1.5);
    QCOMPARE(iconSpy.count(), 1);
    QCOMPARE(nameSpy.count(), 1);
    QCOMPARE(tooltipSpy.count(), 1);
    QCOMPARE(versionSpy.count(), 1);
    QCOMPARE(activeSpy.count(), 1);
    QCOMPARE(ratioSpy.count(), 1);
}

void widget_test::configMapsAreCreatedFromAssignedConfig()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(loadMetadata(dir.path(), QStringLiteral("local-config-widget"), QJsonObject{
        {QStringLiteral("Config"), QStringLiteral("LocalOnly")}
    }));
    Config localConfig(QVariantMap{
        {QStringLiteral("name"), QStringLiteral("dock")},
        {QStringLiteral("enabled"), true}
    }, static_cast<QObject *>(nullptr));

    widget.setConfig(&localConfig);

    QVERIFY(widget.config() == &localConfig);
    QVERIFY(widget.qmlLocalConfig() != nullptr);
    QCOMPARE(widget.qmlLocalConfig()->value(QStringLiteral("name")).toString(), QStringLiteral("dock"));
    QVERIFY(widget.qmlGlobalConfig() == widget.qmlLocalConfig());
    QVERIFY(widget.globalConfig() == &localConfig);
}

void widget_test::containerChangesPropagateOrientation()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(loadMetadata(dir.path(), QStringLiteral("demo-widget")));
    WidgetContainer first(loadMetadata(dir.path(), QStringLiteral("first-container")));
    WidgetContainer second(loadMetadata(dir.path(), QStringLiteral("second-container")));
    QSignalSpy containerSpy(&widget, &Widget::containerChanged);
    QSignalSpy orientationSpy(&widget, &Widget::orientationChanged);

    first.setOrientation(Types::Vertical);
    widget.setContainer(&first);
    QCOMPARE(widget.container(), &first);
    QCOMPARE(widget.orientation(), Types::Vertical);
    QCOMPARE(containerSpy.count(), 1);
    QCOMPARE(containerSpy.first().at(0).toString(), QString());
    QCOMPARE(containerSpy.first().at(1).toString(), QStringLiteral("first-container"));

    first.setOrientation(Types::Horizontal);
    QCOMPARE(widget.orientation(), Types::Horizontal);

    second.setOrientation(Types::Vertical);
    widget.setContainer(&second);
    QCOMPARE(widget.container(), &second);
    QCOMPARE(containerSpy.count(), 2);
    QCOMPARE(containerSpy.at(1).at(0).toString(), QStringLiteral("first-container"));
    QCOMPARE(containerSpy.at(1).at(1).toString(), QStringLiteral("second-container"));

    first.setOrientation(Types::Vertical);
    QCOMPARE(widget.orientation(), Types::Vertical);
    QVERIFY(orientationSpy.count() >= 2);
}

void widget_test::visibleReflectsAssignedState()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(loadMetadata(dir.path(), QStringLiteral("visible-widget")));
    QSignalSpy visibleSpy(&widget, &Widget::visibleChanged);

    QVERIFY(widget.visible());

    widget.setVisible(false);
    QVERIFY(!widget.visible());
    QCOMPARE(visibleSpy.count(), 1);

    widget.setVisible(false);
    QCOMPARE(visibleSpy.count(), 1);

    widget.setVisible(true);
    QVERIFY(widget.visible());
    QCOMPARE(visibleSpy.count(), 2);
}

void widget_test::requestHostFocusEmitsForEveryCall()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(loadMetadata(dir.path(), QStringLiteral("focus-request-widget")));
    QSignalSpy requestSpy(&widget, &Widget::hostFocusRequested);

    const int methodIndex = widget.metaObject()->indexOfMethod("requestHostFocus()");
    QVERIFY(methodIndex >= 0);
    QCOMPARE(widget.metaObject()->method(methodIndex).methodType(), QMetaMethod::Method);

    QVERIFY(QMetaObject::invokeMethod(&widget, "requestHostFocus", Qt::DirectConnection));
    widget.requestHostFocus();

    QCOMPARE(requestSpy.count(), 2);
    QCOMPARE(qvariant_cast<Widget *>(requestSpy.at(0).at(0)), &widget);
    QCOMPARE(qvariant_cast<Widget *>(requestSpy.at(1).at(0)), &widget);
}

QTEST_GUILESS_MAIN(widget_test)

#include "test-widget.moc"
