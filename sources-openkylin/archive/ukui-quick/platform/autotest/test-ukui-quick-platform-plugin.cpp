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

#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QScopedPointer>
#include <QTemporaryDir>
#include <QtQml>
#include <QtTest>

#include "ukui-quick-platform-plugin.h"

namespace {

constexpr auto PlatformUri = "org.ukui.quick.platform";

}

class ukui_quick_platform_plugin_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void registerTypesExposesExpectedQmlTypes();
    void registerTypesRegistersExpectedMetaTypes();

private:
    void importModuleIntoProcess();
};

void ukui_quick_platform_plugin_test::initTestCase()
{
    importModuleIntoProcess();
}

void ukui_quick_platform_plugin_test::importModuleIntoProcess()
{
    QTemporaryDir importDir;
    QVERIFY(importDir.isValid());

    const QString moduleDir = importDir.filePath(QStringLiteral("org/ukui/quick/platform"));
    QVERIFY(QDir().mkpath(moduleDir));

    QFile qmldir(moduleDir + QStringLiteral("/qmldir"));
    QVERIFY(qmldir.open(QIODevice::WriteOnly | QIODevice::Truncate));
    qmldir.write("module org.ukui.quick.platform\nplugin ukui-quick-platform-plugin\n");
    qmldir.close();

    const QString pluginPath = QFileInfo(QCoreApplication::applicationDirPath() + QStringLiteral("/../libukui-quick-platform-plugin.so")).canonicalFilePath();
    QVERIFY2(!pluginPath.isEmpty(), "Platform plugin library should be built before running the import test");
    QVERIFY(QFile::link(pluginPath, moduleDir + QStringLiteral("/libukui-quick-platform-plugin.so")));

    QQmlEngine engine;
    engine.addImportPath(importDir.path());
    QQmlComponent component(&engine);
    component.setData(R"(
        import QtQml 2.15
        import org.ukui.quick.platform 1.0

        QtObject {
            property int tooltipType: WindowType.ToolTip
        }
    )",
                      QUrl());

    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    QScopedPointer<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(component.errorString()));
    QVERIFY(object->property("tooltipType").toInt() >= 0);
}

void ukui_quick_platform_plugin_test::registerTypesExposesExpectedQmlTypes()
{
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "Settings") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "AppLauncher") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "UKUIWindow") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "WindowManager") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "DtThemeDefinition") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "GlobalTheme") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "GradientColor") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "ShadowData") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "RectRegion") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "Region") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "ApplicationIconProxy") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "SlideWindow") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "DtColorType") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "Theme") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "WindowType") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "Decoration") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "WindowState") >= 0);
    QVERIFY(qmlTypeId(PlatformUri, 1, 0, "WindowProxy") >= 0);
}

void ukui_quick_platform_plugin_test::registerTypesRegistersExpectedMetaTypes()
{
    QVERIFY(QMetaType::type("GradientColor") != QMetaType::UnknownType);
    QVERIFY(QMetaType::type("ShadowData") != QMetaType::UnknownType);
    QVERIFY(QMetaType::type("DtColorType") != QMetaType::UnknownType);
    QVERIFY(QMetaType::type("Region *") != QMetaType::UnknownType);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QGuiApplication app(argc, argv);
    ukui_quick_platform_plugin_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-ukui-quick-platform-plugin.moc"
