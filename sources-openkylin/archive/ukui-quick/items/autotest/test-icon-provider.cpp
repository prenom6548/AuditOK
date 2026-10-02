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
#include <QPixmap>
#include <QTemporaryDir>
#include <QtTest>

#include "icon-provider.h"

using namespace UkuiQuick;

namespace {

QString writeSvgIcon(QTemporaryDir &dir, const QString &name, const QString &color)
{
    const QString iconPath = dir.filePath(name + QStringLiteral(".svg"));
    QFile file(iconPath);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    file.write(QStringLiteral(
                   "<svg xmlns='http://www.w3.org/2000/svg' width='16' height='16'>"
                   "<rect width='16' height='16' fill='%1'/>"
                   "</svg>")
                   .arg(color)
                   .toUtf8());
    file.close();
    return iconPath;
}

QColor centerColor(const QPixmap &pixmap)
{
    const QImage image = pixmap.toImage();
    return image.pixelColor(image.width() / 2, image.height() / 2);
}

}

class icon_provider_test : public QObject
{
    Q_OBJECT

private slots:
    void requestPixmapUsesRequestedSize();
    void emptyRequestedSizeFallsBackToDefaultSize();
    void missingIconFallsBackToDefaultIcon();
};

void icon_provider_test::requestPixmapUsesRequestedSize()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    IconProvider provider;
    QSize actualSize;
    const QString iconPath = writeSvgIcon(dir, QStringLiteral("scalable-green"), QStringLiteral("#00ff00"));

    const QPixmap pixmap = provider.requestPixmap(iconPath, &actualSize, QSize(24, 24));

    QVERIFY(!pixmap.isNull());
    QCOMPARE(pixmap.size(), QSize(24, 24));
    QCOMPARE(actualSize, QSize(24, 24));
    QCOMPARE(centerColor(pixmap), QColor(QStringLiteral("#00ff00")));
}

void icon_provider_test::emptyRequestedSizeFallsBackToDefaultSize()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    IconProvider provider;
    QSize actualSize;
    const QString iconPath = writeSvgIcon(dir, QStringLiteral("scalable-blue"), QStringLiteral("#0000ff"));

    const QPixmap pixmap = provider.requestPixmap(iconPath, &actualSize, QSize());

    QVERIFY(!pixmap.isNull());
    QCOMPARE(pixmap.size(), QSize(128, 128));
    QCOMPARE(actualSize, QSize(128, 128));
    QCOMPARE(centerColor(pixmap), QColor(QStringLiteral("#0000ff")));
}

void icon_provider_test::missingIconFallsBackToDefaultIcon()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    IconProvider provider;

    const QPixmap pixmap = provider.requestPixmap(dir.filePath(QStringLiteral("missing.svg")), nullptr, QSize(24, 24));

    QVERIFY(!pixmap.isNull());
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QApplication app(argc, argv);
    icon_provider_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-icon-provider.moc"
