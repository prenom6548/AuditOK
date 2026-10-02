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

QList<WidgetExtension *> g_widgets;
QVariantMap g_receivedData;

class FakeWidgetExtension : public WidgetExtension
{
public:
    FakeWidgetExtension(const QString &id, WidgetMetadata::TypeValue type, WidgetMetadata::FlagValue flag)
        : m_metadata({
              {WidgetMetadata::Id, id},
              {WidgetMetadata::Icon, QStringLiteral("icon-%1").arg(id)},
              {WidgetMetadata::Name, QStringLiteral("name-%1").arg(id)},
              {WidgetMetadata::Tooltip, QStringLiteral("tooltip-%1").arg(id)},
              {WidgetMetadata::Version, QStringLiteral("1.0.%1").arg(id)},
              {WidgetMetadata::Description, QStringLiteral("description-%1").arg(id)},
              {WidgetMetadata::Main, QStringLiteral("main-%1").arg(id)},
              {WidgetMetadata::Type, QVariant::fromValue(type)},
              {WidgetMetadata::Flag, QVariant::fromValue(flag)}
          })
    {
    }

    MetadataMap metadata() const override
    {
        return m_metadata;
    }

    QVariantMap data() override
    {
        return {{QStringLiteral("payload"), m_metadata.value(WidgetMetadata::Id)}};
    }

    void receive(const QVariantMap &data) override
    {
        g_receivedData = data;
    }

    void publishUpdate()
    {
        Q_EMIT dataUpdated();
    }

private:
    MetadataMap m_metadata;
};

} // namespace

#define UKUI_MENU_MENU_EXTENSION_LOADER_H
namespace UkuiMenu {
class MenuExtensionLoader
{
public:
    static MenuExtensionLoader *instance()
    {
        static MenuExtensionLoader loader;
        return &loader;
    }

    QList<WidgetExtension *> widgets() const
    {
        return g_widgets;
    }
};
}

#define private public
#include "../../../src/extension/widget-extension-model.h"
#undef private

#include "../../../src/extension/widget-extension.cpp"
#include "../../../src/extension/widget-extension-model.cpp"

class widgetExtensionModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test_modelDataAndRoles();
    void test_widgetAtAndNotifyBounds();
    void test_widgetDataUpdatedEmitsDataChangedForSourceRow();
};

void widgetExtensionModel_test::init()
{
    g_receivedData.clear();
    g_widgets = {
        new FakeWidgetExtension(QStringLiteral("one"), WidgetMetadata::Widget, WidgetMetadata::Normal),
        new FakeWidgetExtension(QStringLiteral("two"), WidgetMetadata::Button, WidgetMetadata::OnlyFullScreen)
    };
}

void widgetExtensionModel_test::cleanup()
{
    qDeleteAll(g_widgets);
    g_widgets.clear();
}

void widgetExtensionModel_test::test_modelDataAndRoles()
{
    WidgetExtensionModel model;

    QCOMPARE(model.parent(model.index(0, 0)), QModelIndex());
    QCOMPARE(model.rowCount(QModelIndex()), 2);
    QCOMPARE(model.columnCount(QModelIndex()), 1);

    const QModelIndex first = model.index(0, 0);
    QCOMPARE(model.data(first, WidgetMetadata::Id).toString(), QStringLiteral("one"));
    QCOMPARE(model.data(first, WidgetMetadata::Icon).toString(), QStringLiteral("icon-one"));
    QCOMPARE(model.data(first, WidgetMetadata::Name).toString(), QStringLiteral("name-one"));
    QCOMPARE(model.data(first, WidgetMetadata::Tooltip).toString(), QStringLiteral("tooltip-one"));
    QCOMPARE(model.data(first, WidgetMetadata::Version).toString(), QStringLiteral("1.0.one"));
    QCOMPARE(model.data(first, WidgetMetadata::Description).toString(), QStringLiteral("description-one"));
    QCOMPARE(model.data(first, WidgetMetadata::Main).toString(), QStringLiteral("main-one"));
    QCOMPARE(model.data(first, WidgetMetadata::Type).value<WidgetMetadata::TypeValue>(), WidgetMetadata::Widget);
    QCOMPARE(model.data(first, WidgetMetadata::Flag).value<WidgetMetadata::FlagValue>(), WidgetMetadata::Normal);
    QCOMPARE(model.data(first, WidgetMetadata::Data).toMap().value(QStringLiteral("payload")).toString(), QStringLiteral("one"));
    QVERIFY(!model.data(QModelIndex(), WidgetMetadata::Id).isValid());
    QVERIFY(!model.data(first, 999).isValid());

    const QHash<int, QByteArray> roles = model.roleNames();
    QCOMPARE(roles.value(WidgetMetadata::Id), QByteArray("id"));
    QCOMPARE(roles.value(WidgetMetadata::Data), QByteArray("data"));
}

void widgetExtensionModel_test::test_widgetAtAndNotifyBounds()
{
    WidgetExtensionModel model;

    QCOMPARE(model.widgetAt(-1), nullptr);
    QCOMPARE(model.widgetAt(2), nullptr);
    QCOMPARE(model.widgetAt(1), g_widgets.at(1));

    const QVariantMap payload{{QStringLiteral("key"), QStringLiteral("value")}};
    model.notify(1, payload);
    QCOMPARE(g_receivedData, payload);

    g_receivedData.clear();
    model.notify(3, payload);
    QVERIFY(g_receivedData.isEmpty());
}

void widgetExtensionModel_test::test_widgetDataUpdatedEmitsDataChangedForSourceRow()
{
    WidgetExtensionModel model;
    QSignalSpy spy(&model, &WidgetExtensionModel::dataChanged);

    static_cast<FakeWidgetExtension *>(g_widgets.at(1))->publishUpdate();

    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).at(0).toModelIndex(), model.index(1, 0));
    QCOMPARE(spy.at(0).at(1).toModelIndex(), model.index(1, 0));
    QCOMPARE(spy.at(0).at(2).value<QVector<int>>(), QVector<int>{WidgetMetadata::Data});
}

QTEST_MAIN(widgetExtensionModel_test)

#include "widget-extension-model-test.moc"
