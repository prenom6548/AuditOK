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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QtTest>

#include "../../../src/extension/widget-extension.h"

using namespace UkuiMenu;

namespace {

class TestWidgetExtension : public WidgetExtension
{
public:
    explicit TestWidgetExtension(QObject *parent = nullptr)
        : WidgetExtension(parent)
    {
    }

    MetadataMap metadata() const override
    {
        return {};
    }
};

}

namespace UkuiMenu {

class FavoriteFilterModel : public QObject
{
    Q_OBJECT
public:
    static FavoriteFilterModel *instance()
    {
        static FavoriteFilterModel model;
        return &model;
    }
};

class FolderModel : public QObject
{
    Q_OBJECT
public:
    static FolderModel &instance()
    {
        static FolderModel model;
        return model;
    }
};

}

#define FAVORITEFILTERMODEL_H
#define UKUI_MENU_FOLDER_MODEL_H
#include "../../../src/extension/favorite/favorite-widget.cpp"

class favoriteWidget_test : public QObject
{
    Q_OBJECT

private slots:
    void test_widgetExtensionBaseDefaults();
    void test_basicMetadataAndData();
    void test_receive_isSafeNoop();
};

void favoriteWidget_test::test_widgetExtensionBaseDefaults()
{
    QObject parent;
    TestWidgetExtension extension(&parent);

    QCOMPARE(extension.parent(), &parent);
    QCOMPARE(extension.index(), -1);
    QCOMPARE(extension.data(), QVariantMap());
    QCOMPARE(extension.metadata(), MetadataMap());

    extension.receive(QVariantMap{{QStringLiteral("ignored"), true}});
    QVERIFY(true);
}

void favoriteWidget_test::test_basicMetadataAndData()
{
    FavoriteWidget widget;

    QCOMPARE(widget.index(), 0);

    const MetadataMap metadata = widget.metadata();
    QCOMPARE(metadata.value(WidgetMetadata::Id).toString(), QStringLiteral("favorite"));
    QCOMPARE(metadata.value(WidgetMetadata::Icon).toString(), QStringLiteral("non-starred-symbolic"));
    QCOMPARE(metadata.value(WidgetMetadata::Name).toString(), QStringLiteral("Favorite"));
    QCOMPARE(metadata.value(WidgetMetadata::Tooltip).toString(), QStringLiteral("favorite"));
    QCOMPARE(metadata.value(WidgetMetadata::Version).toString(), QStringLiteral("1.0.0"));
    QCOMPARE(metadata.value(WidgetMetadata::Description).toString(), QStringLiteral("favorite"));
    QCOMPARE(metadata.value(WidgetMetadata::Main).toString(), QStringLiteral("qrc:///qml/extensions/FavoriteExtension.qml"));
    QCOMPARE(metadata.value(WidgetMetadata::Type).value<WidgetMetadata::TypeValue>(), WidgetMetadata::Widget);
    QCOMPARE(metadata.value(WidgetMetadata::Flag).value<WidgetMetadata::FlagValue>(), WidgetMetadata::Normal);

    const QVariantMap data = widget.data();
    QVERIFY(data.contains(QStringLiteral("favoriteAppsModel")));
    QVERIFY(data.contains(QStringLiteral("folderModel")));
    QCOMPARE(data.value(QStringLiteral("favoriteAppsModel")).value<QObject *>(),
             static_cast<QObject *>(FavoriteFilterModel::instance()));
    QCOMPARE(data.value(QStringLiteral("folderModel")).value<QObject *>(),
             static_cast<QObject *>(&FolderModel::instance()));
}

void favoriteWidget_test::test_receive_isSafeNoop()
{
    FavoriteWidget widget;
    widget.receive(QVariantMap{{QStringLiteral("key"), 1}});
    QVERIFY(true);
}

QTEST_MAIN(favoriteWidget_test)

#include "favorite-widget-test.moc"
