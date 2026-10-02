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
#include <QQmlComponent>
#include <QQmlEngine>
#include <memory>
#include <QtTest>
#include <qqml.h>

#include "ukui-quick-items-plugin.h"

class ukui_quick_items_plugin_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void registerTypesAndInitializeEngine();
    void registeredTypesCanBeCreatedFromQml();

private:
    std::unique_ptr<UkuiQuickItemsPlugin> m_plugin;
    std::unique_ptr<QQmlEngine> m_engine;
};

void ukui_quick_items_plugin_test::initTestCase()
{
    m_plugin = std::make_unique<UkuiQuickItemsPlugin>();
    m_engine = std::make_unique<QQmlEngine>();

    m_plugin->registerTypes("org.ukui.quick.items");
    m_plugin->initializeEngine(m_engine.get(), "org.ukui.quick.items");
}

void ukui_quick_items_plugin_test::registerTypesAndInitializeEngine()
{
    QVERIFY(m_engine->imageProvider("theme") != nullptr);
    QVERIFY(qmlTypeId("org.ukui.quick.items", 1, 0, "GradientBorderItem") > 0);
    QVERIFY(qmlTypeId("org.ukui.quick.items", 1, 0, "ContentWindow") > 0);
    QVERIFY(qmlTypeId("org.ukui.quick.items", 1, 0, "WindowBlurBehind") > 0);
    QVERIFY(qmlTypeId("org.ukui.quick.items", 1, 0, "ShadowedTexture") > 0);
}

void ukui_quick_items_plugin_test::registeredTypesCanBeCreatedFromQml()
{
    QQmlEngine engine;
    engine.setImportPathList({});

    QQmlComponent component(&engine);
    component.setData(R"(
        import QtQuick 2.15
        import org.ukui.quick.items 1.0

        GradientBorderItem {
            width: 32
            height: 16
        }
    )", QUrl());

    QObject *object = component.create();
    QVERIFY2(object != nullptr, qPrintable(component.errorString()));
    delete object;
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");
    qputenv("GSETTINGS_BACKEND", "memory");

    QGuiApplication app(argc, argv);
    ukui_quick_items_plugin_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-ukui-quick-items-plugin.moc"
