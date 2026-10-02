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
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

#include "widget-content.h"
#include "widget-metadata.h"

using namespace UkuiQuick;

namespace {

QString createWidgetPackage(const QString &basePath, const QString &id, const QJsonObject &contents)
{
    const QString root = QDir(basePath).filePath(id);
    QDir().mkpath(root);

    QFile file(QDir(root).filePath(QStringLiteral("metadata.json")));
    file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text);
    file.write(QJsonDocument(QJsonObject{
        {QStringLiteral("Id"), id},
        {QStringLiteral("Name"), id},
        {QStringLiteral("Contents"), contents}
    }).toJson());
    file.close();

    return root;
}

}

class widget_content_test : public QObject
{
    Q_OBJECT

private slots:
    void exposesConfiguredContentFields();
    void fileUrlUsesQrcSchemeForQrcContent();
};

void widget_content_test::exposesConfiguredContentFields()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString root = createWidgetPackage(dir.path(), QStringLiteral("demo-content"), QJsonObject{
        {QStringLiteral("Main"), QStringLiteral("ui/Main.qml")},
        {QStringLiteral("Data"), QStringLiteral("data.json")},
        {QStringLiteral("I18n"), QStringLiteral("translations/demo")},
        {QStringLiteral("Config"), QStringLiteral("LocalOnly")},
        {QStringLiteral("Plugin"), QStringLiteral("libdemo.so")},
        {QStringLiteral("PluginVersion"), QStringLiteral("1.0")},
        {QStringLiteral("PluginPreload"), true}
    });
    WidgetMetadata metadata(root);
    WidgetContent content(metadata);

    QVERIFY(metadata.isValid());
    QCOMPARE(content.main(), QStringLiteral("ui/Main.qml"));
    QCOMPARE(content.data(), QStringLiteral("data.json"));
    QCOMPARE(content.i18n(), QStringLiteral("translations/demo"));
    QCOMPARE(content.config(), QStringLiteral("LocalOnly"));
    QCOMPARE(content.plugin(), QStringLiteral("libdemo.so"));
    QCOMPARE(content.pluginVersion(), QStringLiteral("1.0"));
    QVERIFY(content.pluginPreload());
    QCOMPARE(content.configPolicy(), WidgetContent::LocalOnly);
    QCOMPARE(content.rootPath(), QDir(root).absolutePath());
    QCOMPARE(content.filePath(WidgetContent::Main), QDir(root).filePath(QStringLiteral("ui/Main.qml")));
    QCOMPARE(content.filePath(WidgetContent::I18n), QDir(root).filePath(QStringLiteral("translations/demo")));
    QCOMPARE(content.filePath(WidgetContent::Plugin), QDir(root).filePath(QStringLiteral("libdemo.so")));
    QVERIFY(content.filePath(WidgetContent::Config).isEmpty());
    QCOMPARE(content.fileUrl(WidgetContent::Main), QUrl::fromLocalFile(QDir(root).filePath(QStringLiteral("ui/Main.qml"))));
    QVERIFY(!content.fileUrl(WidgetContent::Config).isValid());
}

void widget_content_test::fileUrlUsesQrcSchemeForQrcContent()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString root = createWidgetPackage(dir.path(), QStringLiteral("demo-qrc"), QJsonObject{
        {QStringLiteral("Main"), QStringLiteral(":/ukui/widgets/Main.qml")}
    });
    WidgetMetadata metadata(root);
    WidgetContent content(metadata);

    QCOMPARE(content.fileUrl(WidgetContent::Main), QUrl(QStringLiteral("qrc:/ukui/widgets/Main.qml")));
}

QTEST_GUILESS_MAIN(widget_content_test)

#include "test-widget-content.moc"
