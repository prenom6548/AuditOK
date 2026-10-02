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
#include <QImage>
#include <QIcon>
#include <QPixmap>
#include <QTemporaryDir>
#include <QtTest>

#include "icon-helper.h"
#include "icon-theme-test-utils_p.h"

using namespace UkuiQuick;

namespace {

QColor centerColor(const QIcon &icon)
{
    const QImage image = icon.pixmap(16, 16).toImage();
    return image.pixelColor(image.width() / 2, image.height() / 2);
}

}

class icon_helper_test : public QObject
{
    Q_OBJECT

private slots:
    void toLocalPathRecognizesSupportedLocalForms();
    void toLocalPathRejectsUnsupportedForms();
    void fileTypePredicatesClassifyUrls();
    void loadPixmapLoadsExistingImagesOnly();
    void emptyIdUsesDefaultIcon();
    void loadIconUsesLocalFileAndFallsBackWhenMissing();
    void defaultIconFallsBackToResourceWhenThemeMissing();
    void themeLookupRecognizesCustomThemeIcons();
    void themeLookupRejectsMissingIconName();
};

void icon_helper_test::toLocalPathRecognizesSupportedLocalForms()
{
    QCOMPARE(IconHelper::toLocalPath(QUrl::fromLocalFile("/tmp/test-icon.png")), QStringLiteral("/tmp/test-icon.png"));
    QCOMPARE(IconHelper::toLocalPath(QUrl(QStringLiteral("qrc:///res/icon/application-x-desktop.png"))),
             QStringLiteral(":/res/icon/application-x-desktop.png"));
    QCOMPARE(IconHelper::toLocalPath(QUrl(QStringLiteral("/absolute/icon.png"))), QStringLiteral("/absolute/icon.png"));
    QCOMPARE(IconHelper::toLocalPath(QUrl(QStringLiteral(":/res/icon/application-x-desktop.png"))),
             QStringLiteral(":/res/icon/application-x-desktop.png"));
}

void icon_helper_test::toLocalPathRejectsUnsupportedForms()
{
    QCOMPARE(IconHelper::toLocalPath(QUrl()), QString());
    QCOMPARE(IconHelper::toLocalPath(QUrl(QStringLiteral("relative/icon.png"))), QString());
    QCOMPARE(IconHelper::toLocalPath(QUrl(QStringLiteral("https://example.com/icon.png"))), QString());
    QCOMPARE(IconHelper::toLocalPath(QUrl(QStringLiteral("ftp://example.com/icon.png"))), QString());
}

void icon_helper_test::fileTypePredicatesClassifyUrls()
{
    QVERIFY(IconHelper::isLocalFile(QUrl::fromLocalFile("/tmp/test-icon.png")));
    QVERIFY(IconHelper::isLocalFile(QUrl(QStringLiteral("qrc:///res/icon/application-x-desktop.png"))));
    QVERIFY(!IconHelper::isLocalFile(QUrl(QStringLiteral("relative/icon.png"))));
    QVERIFY(!IconHelper::isLocalFile(QUrl(QStringLiteral("https://example.com/icon.png"))));

    QVERIFY(IconHelper::isRemoteServerFile(QUrl(QStringLiteral("http://example.com/icon.png"))));
    QVERIFY(IconHelper::isRemoteServerFile(QUrl(QStringLiteral("https://example.com/icon.png"))));
    QVERIFY(!IconHelper::isRemoteServerFile(QUrl(QStringLiteral("ftp://example.com/icon.png"))));
    QVERIFY(!IconHelper::isRemoteServerFile(QUrl()));
}

void icon_helper_test::loadPixmapLoadsExistingImagesOnly()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString imagePath = dir.filePath(QStringLiteral("icon.png"));
    QImage image(4, 4, QImage::Format_ARGB32);
    image.fill(Qt::red);
    QVERIFY(image.save(imagePath));

    QPixmap pixmap;
    QVERIFY(IconHelper::loadPixmap(imagePath, pixmap));
    QVERIFY(!pixmap.isNull());

    QPixmap missingPixmap;
    QVERIFY(!IconHelper::loadPixmap(dir.filePath(QStringLiteral("missing.png")), missingPixmap));
    QVERIFY(missingPixmap.isNull());
}

void icon_helper_test::emptyIdUsesDefaultIcon()
{
    Test::IconThemeScope guard;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(Test::writeThemeIcon(&dir, QStringLiteral("codex-theme"),
                                 QStringLiteral("application-x-desktop"), QColor(Qt::blue)));
    QIcon::setThemeSearchPaths({dir.path()});
    QIcon::setThemeName(QStringLiteral("codex-theme"));

    const QIcon icon = IconHelper::loadIcon(QString());

    QVERIFY(!icon.isNull());
    QCOMPARE(centerColor(icon), QColor(Qt::blue));
}

void icon_helper_test::loadIconUsesLocalFileAndFallsBackWhenMissing()
{
    Test::IconThemeScope guard;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString localPath = dir.filePath(QStringLiteral("local.png"));
    QImage localImage(4, 4, QImage::Format_ARGB32);
    localImage.fill(Qt::green);
    QVERIFY(localImage.save(localPath));

    QVERIFY(Test::writeThemeIcon(&dir, QStringLiteral("codex-theme"),
                                 QStringLiteral("application-x-desktop"), QColor(Qt::blue)));
    QIcon::setThemeSearchPaths({dir.path()});
    QIcon::setThemeName(QStringLiteral("codex-theme"));

    const QIcon localIcon = IconHelper::loadIcon(localPath);
    const QIcon fallbackIcon = IconHelper::loadIcon(dir.filePath(QStringLiteral("missing.png")));

    QVERIFY(!localIcon.isNull());
    QVERIFY(!fallbackIcon.isNull());
    QCOMPARE(centerColor(localIcon), QColor(Qt::green));
    QCOMPARE(centerColor(fallbackIcon), QColor(Qt::blue));
}

void icon_helper_test::defaultIconFallsBackToResourceWhenThemeMissing()
{
    Test::IconThemeScope guard;
    QIcon::setThemeSearchPaths({});
    QIcon::setThemeName(QStringLiteral("missing-codex-theme"));

    const QIcon icon = IconHelper::getDefaultIcon();

    QVERIFY(!icon.isNull());
}

void icon_helper_test::themeLookupRecognizesCustomThemeIcons()
{
    Test::IconThemeScope guard;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(Test::writeThemeIcon(&dir, QStringLiteral("codex-theme"),
                                 QStringLiteral("ukui-quick-autotest-themed"), QColor(Qt::yellow)));
    QIcon::setThemeSearchPaths({dir.path()});
    QIcon::setThemeName(QStringLiteral("codex-theme"));

    QVERIFY(IconHelper::isThemeIcon(QStringLiteral("ukui-quick-autotest-themed")));

    const QIcon icon = IconHelper::loadIcon(QStringLiteral("ukui-quick-autotest-themed"));
    QVERIFY(!icon.isNull());
    QCOMPARE(centerColor(icon), QColor(Qt::yellow));
}

void icon_helper_test::themeLookupRejectsMissingIconName()
{
    QVERIFY(!IconHelper::isThemeIcon(QStringLiteral("ukui-quick-autotest-missing-icon-name")));
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QApplication app(argc, argv);
    icon_helper_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-icon-helper.moc"
