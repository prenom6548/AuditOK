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
#include "../../../src/libappdata/app-category-plugin.h"
#include "../../../src/libappdata/app-category-model.h"
#include "../../../src/libappdata/recently-installed-model.h"
#include "../../../src/utils/event-track.h"

using namespace UkuiMenu;

namespace {

QStandardItemModel *g_categorySourceModel = nullptr;
QStandardItemModel *g_recentlySourceModel = nullptr;
QString g_lastEventName;
QString g_lastEventPage;
QVariantMap g_lastEventMap;

QStandardItem *createPluginItem(const QString &name, const QString &group)
{
    auto *item = new QStandardItem(name);
    item->setData(name, DataEntity::Name);
    item->setData(group, DataEntity::Group);
    return item;
}

}

namespace UkuiMenu {

AppCategoryModel::AppCategoryModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    if (g_categorySourceModel) {
        QSortFilterProxyModel::setSourceModel(g_categorySourceModel);
    }
}

AppCategoryModel::Mode AppCategoryModel::mode() const
{
    return m_mode;
}

void AppCategoryModel::setMode(AppCategoryModel::Mode mode)
{
    if (m_mode == mode) {
        return;
    }

    m_mode = mode;
    invalidateFilter();
}

QVariant AppCategoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (role == DataEntity::Group && m_mode == All) {
        return QString();
    }

    return QSortFilterProxyModel::data(index, role);
}

bool AppCategoryModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_row);
    Q_UNUSED(source_parent);
    return m_mode != All;
}

bool AppCategoryModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return source_left.row() < source_right.row();
}

QString AppCategoryModel::getFirstLatterUpper(const QString &pinyinName)
{
    return pinyinName.left(1).toUpper();
}

QString AppCategoryModel::getCategoryName(const QString &categories) const
{
    return categories;
}

int AppCategoryModel::getCategoryIndex(const QString &categories) const
{
    Q_UNUSED(categories);
    return 0;
}

QString AppCategoryModel::getCategoryKey(const QString &categories) const
{
    return categories;
}

RecentlyInstalledModel::RecentlyInstalledModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    if (g_recentlySourceModel) {
        QSortFilterProxyModel::setSourceModel(g_recentlySourceModel);
    }
}

bool RecentlyInstalledModel::event(QEvent *event)
{
    return QObject::event(event);
}

QVariant RecentlyInstalledModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (role == DataEntity::Group && m_mode != AppCategoryModel::All) {
        return tr("Recently Installed");
    }

    if (role == DataEntity::RecentInstall) {
        return true;
    }

    return QSortFilterProxyModel::data(index, role);
}

void RecentlyInstalledModel::setMode(AppCategoryModel::Mode mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        invalidateFilter();
    }
}

bool RecentlyInstalledModel::isRecentInstall(const DataEntity &data)
{
    Q_UNUSED(data);
    return true;
}

bool RecentlyInstalledModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_row);
    Q_UNUSED(source_parent);
    return m_mode != AppCategoryModel::All;
}

bool RecentlyInstalledModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return source_left.row() < source_right.row();
}

qreal RecentlyInstalledModel::getDegree(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return 0;
}

void RecentlyInstalledModel::executeInvalidate()
{
    invalidate();
}

EventTrack *EventTrack::qmlAttachedProperties(QObject *object)
{
    return new EventTrack(object);
}

EventTrack *EventTrack::instance()
{
    static EventTrack instance;
    return &instance;
}

EventTrack::EventTrack(QObject *parent)
    : QObject(parent)
{
}

void EventTrack::sendClickEvent(const QString &event, const QString &page, const QVariantMap &map)
{
    g_lastEventName = event;
    g_lastEventPage = page;
    g_lastEventMap = map;
}

void EventTrack::sendDefaultEvent(const QString &event, const QString &page, const QVariantMap &map)
{
    Q_UNUSED(event);
    Q_UNUSED(page);
    Q_UNUSED(map);
}

void EventTrack::sendSearchEvent(const QString &event, const QString &page, const QString &content)
{
    Q_UNUSED(event);
    Q_UNUSED(page);
    Q_UNUSED(content);
}

}

class appCategoryPlugin_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void test_defaultState_usesAllApplicationsView();
    void test_letterAction_updatesTitleLabelsAndEvent();
    void test_categoryAction_updatesTitleLabelsAndEvent();

private:
    QStandardItemModel *m_categoryModel = nullptr;
    QStandardItemModel *m_recentModel = nullptr;
};

void appCategoryPlugin_test::init()
{
    g_lastEventName.clear();
    g_lastEventPage.clear();
    g_lastEventMap.clear();

    m_categoryModel = new QStandardItemModel;
    m_recentModel = new QStandardItemModel;
    m_categoryModel->setColumnCount(1);
    m_recentModel->setColumnCount(1);

    m_categoryModel->appendRow(createPluginItem(QStringLiteral("Alpha"), QStringLiteral("A")));
    m_categoryModel->appendRow(createPluginItem(QStringLiteral("Beta"), QStringLiteral("B")));
    m_categoryModel->appendRow(createPluginItem(QStringLiteral("Again"), QStringLiteral("A")));
    m_recentModel->appendRow(createPluginItem(QStringLiteral("Recent"), QStringLiteral("Recent")));

    g_categorySourceModel = m_categoryModel;
    g_recentlySourceModel = m_recentModel;
}

void appCategoryPlugin_test::cleanup()
{
    delete m_categoryModel;
    delete m_recentModel;
    g_categorySourceModel = nullptr;
    g_recentlySourceModel = nullptr;
}

void appCategoryPlugin_test::test_defaultState_usesAllApplicationsView()
{
    AppCategoryPlugin plugin;

    QCOMPARE(plugin.group(), AppListPluginGroup::Display);
    QCOMPARE(plugin.name(), QStringLiteral("AppCategoryPlugin"));
    QCOMPARE(plugin.actions().size(), 3);
    QCOMPARE(plugin.title(), plugin.actions().at(0)->text());
    QCOMPARE(plugin.labelBottle()->visible(), false);
    QCOMPARE(plugin.labelBottle()->column(), 0);
}

void appCategoryPlugin_test::test_letterAction_updatesTitleLabelsAndEvent()
{
    AppCategoryPlugin plugin;

    plugin.actions().at(1)->trigger();

    QCOMPARE(plugin.title(), plugin.actions().at(1)->text());
    QCOMPARE(plugin.labelBottle()->visible(), true);
    QCOMPARE(plugin.labelBottle()->column(), 5);
    QCOMPARE(plugin.labelBottle()->labels().size(), 3);
    QCOMPARE(plugin.labelBottle()->labels().at(0).displayName(), QStringLiteral("document-open-recent-symbolic"));
    QCOMPARE(plugin.labelBottle()->labels().at(1).labelName(), QStringLiteral("A"));
    QCOMPARE(plugin.labelBottle()->labels().at(2).labelName(), QStringLiteral("B"));
    QCOMPARE(g_lastEventName, QStringLiteral("switch_app_view"));
    QCOMPARE(g_lastEventPage, QStringLiteral("AppView"));
    QCOMPARE(g_lastEventMap.value(QStringLiteral("viewName")).toString(), QStringLiteral("letter"));
}

void appCategoryPlugin_test::test_categoryAction_updatesTitleLabelsAndEvent()
{
    AppCategoryPlugin plugin;

    plugin.actions().at(2)->trigger();

    QCOMPARE(plugin.title(), plugin.actions().at(2)->text());
    QCOMPARE(plugin.labelBottle()->visible(), true);
    QCOMPARE(plugin.labelBottle()->column(), 2);
    QCOMPARE(plugin.labelBottle()->labels().size(), 3);
    QCOMPARE(plugin.labelBottle()->labels().at(0).type(), LabelItem::Icon);
    QCOMPARE(g_lastEventMap.value(QStringLiteral("viewName")).toString(), QStringLiteral("category"));
}

QTEST_MAIN(appCategoryPlugin_test)

#include "app-category-plugin-test.moc"
