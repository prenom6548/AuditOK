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

#include <QTemporaryDir>
#include <QtTest>

#include "config-loader.h"

using namespace UkuiQuick;

class config_loader_test : public QObject
{
    Q_OBJECT

private slots:
    void fileNameReflectsIdAndType();
    void rejectsPathComponents();
    void fullFileNameReflectsDomainAndApplication();
};

void config_loader_test::fileNameReflectsIdAndType()
{
    QCOMPARE(ConfigLoader::getConfigFileName(QString(), ConfigLoader::Json), QString());
    QCOMPARE(ConfigLoader::getConfigFileName(QStringLiteral("panel"), ConfigLoader::Json), QStringLiteral("panel.json"));
    QCOMPARE(ConfigLoader::getConfigFileName(QStringLiteral("panel"), ConfigLoader::Ini), QStringLiteral("panel.conf"));
}

void config_loader_test::rejectsPathComponents()
{
    const QStringList invalidValues = {
        QString(),
        QStringLiteral("."),
        QStringLiteral(".."),
        QStringLiteral("/tmp/panel"),
        QStringLiteral("nested/panel"),
        QStringLiteral("nested\\panel")
    };

    for (const QString &value : invalidValues) {
        QCOMPARE(ConfigLoader::getConfigFileName(value, ConfigLoader::Json), QString());
        QCOMPARE(ConfigLoader::getFullFileName(value, ConfigLoader::Json, ConfigLoader::Global), QString());
        QCOMPARE(ConfigLoader::getFullFileName(QStringLiteral("panel"), ConfigLoader::Json,
                                               ConfigLoader::Local, value), QString());
    }

    QCOMPARE(ConfigLoader::getConfigFileName(QStringLiteral("a..b"), ConfigLoader::Json),
             QStringLiteral("a..b.json"));
}

void config_loader_test::fullFileNameReflectsDomainAndApplication()
{
    QTemporaryDir homeDir;
    QVERIFY(homeDir.isValid());
    const QByteArray oldHome = qgetenv("HOME");
    qputenv("HOME", QFile::encodeName(homeDir.path()));

    const QString globalPath = homeDir.path() + QStringLiteral("/.config/org.ukui/_ukui-config-global/");
    const QString localPath = homeDir.path() + QStringLiteral("/.config/org.ukui/");

    QCOMPARE(ConfigLoader::globalConfigPath(), globalPath);
    QCOMPARE(ConfigLoader::localConfigPath(), localPath);
    QCOMPARE(ConfigLoader::getFullFileName(QString(), ConfigLoader::Json, ConfigLoader::Global), QString());
    QCOMPARE(ConfigLoader::getFullFileName(QStringLiteral("panel"), ConfigLoader::Json, ConfigLoader::Global),
             globalPath + QStringLiteral("panel.json"));
    QCOMPARE(ConfigLoader::getFullFileName(QStringLiteral("panel"), ConfigLoader::Ini, ConfigLoader::Local, QStringLiteral("demo")),
             localPath + QStringLiteral("demo/panel.conf"));
    QCOMPARE(ConfigLoader::getFullFileName(QStringLiteral("panel"), ConfigLoader::Json, ConfigLoader::Local), QString());

    qputenv("HOME", oldHome);
}

QTEST_GUILESS_MAIN(config_loader_test)

#include "test-config-loader.moc"
