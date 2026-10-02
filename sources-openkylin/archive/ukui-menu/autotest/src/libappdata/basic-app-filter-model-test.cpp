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
#include <QStandardItemModel>

#include "../../../src/data-entity.h"
#include "../../../src/libappdata/basic-app-filter-model.h"
#include "../../../src/libappdata/basic-app-model.h"
#include "../../../src/utils/security-function-control.h"

using namespace UkuiMenu;

namespace {

QStandardItemModel *g_filterSourceModel = nullptr;
QStringList g_visibleApps;

QStandardItem *createFilterItem(const QString &id, const QString &name)
{
    auto *item = new QStandardItem(name);
    item->setData(id, DataEntity::Id);
    item->setData(name, DataEntity::Name);
    DataEntity entity;
    entity.setId(id);
    entity.setName(name);
    item->setData(QVariant::fromValue(entity), DataEntity::Entity);
    return item;
}

}

namespace UkuiMenu {

BasicAppModel *BasicAppModel::instance()
{
    static BasicAppModel model;
    return &model;
}

BasicAppModel::BasicAppModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

const AppDatabaseInterface *BasicAppModel::databaseInterface() const
{
    return nullptr;
}

DataEntity BasicAppModel::appOfIndex(int row) const
{
    return data(index(row, 0), DataEntity::Entity).value<DataEntity>();
}

int BasicAppModel::indexOfApp(const QString &appid) const
{
    if (!g_filterSourceModel) {
        return -1;
    }
    for (int row = 0; row < g_filterSourceModel->rowCount(); ++row) {
        if (g_filterSourceModel->index(row, 0).data(DataEntity::Id).toString() == appid) {
            return row;
        }
    }
    return -1;
}

bool BasicAppModel::getAppById(const QString &appid, DataEntity &app) const
{
    const int row = indexOfApp(appid);
    if (row < 0) {
        return false;
    }
    app = appOfIndex(row);
    return true;
}

int BasicAppModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return g_filterSourceModel ? g_filterSourceModel->rowCount() : 0;
}

int BasicAppModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant BasicAppModel::data(const QModelIndex &index, int role) const
{
    if (!g_filterSourceModel || !index.isValid()) {
        return {};
    }
    return g_filterSourceModel->index(index.row(), 0).data(role);
}

bool BasicAppModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!g_filterSourceModel || !index.isValid()) {
        return false;
    }
    return g_filterSourceModel->setData(g_filterSourceModel->index(index.row(), 0), value, role);
}

QHash<int, QByteArray> BasicAppModel::roleNames() const
{
    return DataEntity::AppRoleNames();
}

void BasicAppModel::onAppAdded(const UkuiMenu::DataEntityVector &apps)
{
    Q_UNUSED(apps);
}

void BasicAppModel::onAppUpdated(const QVector<QPair<UkuiMenu::DataEntity, QVector<int>>> &updates)
{
    Q_UNUSED(updates);
}

void BasicAppModel::onAppDeleted(const QStringList &apps)
{
    Q_UNUSED(apps);
}

SecurityFunctionControl *SecurityFunctionControl::instance()
{
    static SecurityFunctionControl control;
    return &control;
}

SecurityFunctionControl::SecurityFunctionControl(QObject *parent)
    : QObject(parent)
{
}

bool SecurityFunctionControl::disable() const { return false; }
bool SecurityFunctionControl::disableMouseLeftButton() const { return false; }
bool SecurityFunctionControl::disableMouseRightButton() const { return false; }
bool SecurityFunctionControl::disableMouseHover() const { return false; }
QStringList SecurityFunctionControl::appWhiteList() const { return {}; }
QStringList SecurityFunctionControl::appBlackList() const { return {}; }
bool SecurityFunctionControl::disableUninstallAction() const { return false; }
bool SecurityFunctionControl::windowMaximize() const { return true; }
bool SecurityFunctionControl::showFoldButton() const { return true; }
QString SecurityFunctionControl::powerButtonSoleOperation() const { return {}; }
bool SecurityFunctionControl::compareListBySort(QStringList &list1, QStringList &list2) { return list1 == list2; }
void SecurityFunctionControl::onFunctionControlChanged(const QString &name, bool exit) { Q_UNUSED(name); Q_UNUSED(exit); }
bool SecurityFunctionControl::canAppDisplay(const QString &desktop) const
{
    return g_visibleApps.contains(desktop);
}

}

class basicAppFilterModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_filterAcceptsVisibleApps();
    void test_accessors_forwardToSourceModel();

private:
    QStandardItemModel m_sourceModel;
};

void basicAppFilterModel_test::init()
{
    m_sourceModel.clear();
    m_sourceModel.setColumnCount(1);
    m_sourceModel.appendRow(createFilterItem(QStringLiteral("/tmp/a.desktop"), QStringLiteral("Alpha")));
    m_sourceModel.appendRow(createFilterItem(QStringLiteral("/tmp/b.desktop"), QStringLiteral("Beta")));
    g_filterSourceModel = &m_sourceModel;
    g_visibleApps = QStringList() << QStringLiteral("/tmp/a.desktop");
    BasicAppFilterModel::instance()->setSourceModel(BasicAppModel::instance());
    BasicAppFilterModel::instance()->invalidate();
}

void basicAppFilterModel_test::test_filterAcceptsVisibleApps()
{
    QCOMPARE(BasicAppFilterModel::instance()->rowCount(), 1);
    QCOMPARE(BasicAppFilterModel::instance()->data(BasicAppFilterModel::instance()->index(0, 0), DataEntity::Name).toString(),
             QStringLiteral("Alpha"));
}

void basicAppFilterModel_test::test_accessors_forwardToSourceModel()
{
    DataEntity app;
    QVERIFY(BasicAppFilterModel::instance()->getAppById(QStringLiteral("/tmp/a.desktop"), app));
    QCOMPARE(app.name(), QStringLiteral("Alpha"));
    QCOMPARE(BasicAppFilterModel::instance()->indexOfApp(QStringLiteral("/tmp/a.desktop")), 0);
    QCOMPARE(BasicAppFilterModel::instance()->appOfIndex(0).id(), QStringLiteral("/tmp/a.desktop"));
}

QTEST_MAIN(basicAppFilterModel_test)

#include "basic-app-filter-model-test.moc"
