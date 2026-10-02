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

QVariantMap g_notifiedData;
int g_notifiedIndex = -1;

class FakeWidgetExtensionModel : public QAbstractListModel
{
    Q_OBJECT
public:
    static FakeWidgetExtensionModel *instance()
    {
        static FakeWidgetExtensionModel model;
        return &model;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return 3;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid()) {
            return {};
        }

        if (role == WidgetMetadata::Type) {
            const QVector<WidgetMetadata::TypeValue> types{WidgetMetadata::Widget, WidgetMetadata::Button, WidgetMetadata::AppList};
            return QVariant::fromValue(types.at(index.row()));
        }

        if (role == WidgetMetadata::Flag) {
            const QVector<WidgetMetadata::FlagValue> flags{WidgetMetadata::OnlySmallScreen, WidgetMetadata::OnlyFullScreen, WidgetMetadata::Normal};
            return QVariant::fromValue(flags.at(index.row()));
        }

        return {};
    }

    void notify(int index, const QVariantMap &data) const
    {
        g_notifiedIndex = index;
        g_notifiedData = data;
    }
};

} // namespace

#define UKUI_MENU_WIDGET_EXTENSION_MODEL_H
namespace UkuiMenu {
using WidgetExtensionModel = FakeWidgetExtensionModel;
}

#define protected public
#include "../../../src/extension/widget-model.h"
#undef protected

#include "../../../src/extension/widget-extension.cpp"
#include "../../../src/extension/widget-model.cpp"

class widgetModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void test_initAndPropertySetters();
    void test_filterAcceptsMatchingTypeAndFlag();
    void test_sendForwardsToSourceModel();
};

void widgetModel_test::init()
{
    g_notifiedIndex = -1;
    g_notifiedData.clear();
}

void widgetModel_test::test_initAndPropertySetters()
{
    WidgetModel model;

    QCOMPARE(model.sourceModel(), static_cast<QAbstractItemModel *>(FakeWidgetExtensionModel::instance()));
    QCOMPARE(model.types(), WidgetMetadata::Types(WidgetMetadata::Widget));
    QCOMPARE(model.flags(), WidgetMetadata::Flags(WidgetMetadata::Normal));

    model.setTypes(WidgetMetadata::Button);
    QCOMPARE(model.types(), WidgetMetadata::Types(WidgetMetadata::Button));
    model.setTypes(WidgetMetadata::Button);
    QCOMPARE(model.types(), WidgetMetadata::Types(WidgetMetadata::Button));

    model.setFlags(WidgetMetadata::OnlyFullScreen);
    QCOMPARE(model.flags(), WidgetMetadata::Flags(WidgetMetadata::OnlyFullScreen));
    model.setFlags(WidgetMetadata::OnlyFullScreen);
    QCOMPARE(model.flags(), WidgetMetadata::Flags(WidgetMetadata::OnlyFullScreen));
}

void widgetModel_test::test_filterAcceptsMatchingTypeAndFlag()
{
    WidgetModel model;

    model.setTypes(WidgetMetadata::Types(WidgetMetadata::Widget) | WidgetMetadata::Button);
    model.setFlags(WidgetMetadata::OnlyFullScreen);
    QVERIFY(!model.filterAcceptsRow(0, QModelIndex()));
    QVERIFY(model.filterAcceptsRow(1, QModelIndex()));
    QVERIFY(!model.filterAcceptsRow(2, QModelIndex()));

    model.setTypes(WidgetMetadata::AppList);
    model.setFlags(WidgetMetadata::OnlySmallScreen);
    QVERIFY(model.filterAcceptsRow(2, QModelIndex()));
}

void widgetModel_test::test_sendForwardsToSourceModel()
{
    WidgetModel model;
    const QVariantMap payload{{QStringLiteral("command"), QStringLiteral("open")}};

    model.send(7, payload);

    QCOMPARE(g_notifiedIndex, 7);
    QCOMPARE(g_notifiedData, payload);
}

QTEST_MAIN(widgetModel_test)

#include "widget-model-test.moc"
