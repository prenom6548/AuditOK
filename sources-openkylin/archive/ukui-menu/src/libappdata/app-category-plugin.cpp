/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "app-category-plugin.h"
#include "combined-list-model.h"
#include "recently-installed-model.h"
#include "data-entity.h"
#include "event-track.h"
#include "settings.h"

#include <QDebug>

namespace UkuiMenu {

AppCategoryPlugin::AppCategoryPlugin(QObject *parent) : AppListPluginInterface(parent)
    , m_dataModel(new CombinedListModel(this)), m_labelBottle(new LabelBottle(this))
{
    m_categoryModel = new AppCategoryModel(this);
    m_recentlyModel = new RecentlyInstalledModel(this);

    m_dataModel->insertSubModel(m_recentlyModel);
    m_dataModel->insertSubModel(m_categoryModel);

    auto allAction = new QAction(QIcon::fromTheme("view-grid-symbolic"), tr("All Applications"), this);
    auto firstLatterAction = new QAction(QIcon::fromTheme("ukui-capslock-symbolic"), tr("Letter Sort"), this);
    auto categoryAction = new QAction(QIcon::fromTheme("applications-utilities-symbolic"), tr("Category"), this);

    allAction->setCheckable(true);
    firstLatterAction->setCheckable(true);
    categoryAction->setCheckable(true);

    // GCOVR_EXCL_BR_START
    connect(allAction, &QAction::triggered, this, [=] {
        switchTo(AppCategoryModel::All, QStringLiteral("all"));
    });

    connect(firstLatterAction, &QAction::triggered, this, [=] {
        switchTo(AppCategoryModel::FirstLatter, QStringLiteral("letter"));
    });

    connect(categoryAction, &QAction::triggered, this, [=] {
        switchTo(AppCategoryModel::Category, QStringLiteral("category"));
    });

    connect(m_categoryModel, &AppCategoryModel::rowsInserted, this, [=] {
        Q_EMIT labelChanged();
    });
    connect(m_categoryModel, &AppCategoryModel::rowsRemoved, this, [=] {
        Q_EMIT labelChanged();
    });
    connect(m_recentlyModel, &RecentlyInstalledModel::rowsInserted, this, [=] {
        Q_EMIT labelChanged();
    });
    connect(m_recentlyModel, &RecentlyInstalledModel::rowsRemoved, this, [=] {
        Q_EMIT labelChanged();
    });
    connect(m_recentlyModel, &RecentlyInstalledModel::modelReset, this, [=] {
        Q_EMIT labelChanged();
    });
    // GCOVR_EXCL_BR_STOP

    m_actions.append(allAction);
    m_actions.append(firstLatterAction);
    m_actions.append(categoryAction);

    allAction->setChecked(true);
    setTitle(allAction->text());
    m_labelBottle->setColumn(0);
    m_labelBottle->setVisible(false);
    m_modeToAction["all"] = allAction;
    m_modeToAction["letter"] = firstLatterAction;
    m_modeToAction["category"] = categoryAction;

    connect(MenuSetting::instance(), &MenuSetting::changed, this, [=] (const QString& key) {
        if (key == APP_LIST_DISPLAY_MODE) {
            syncViewFromGSettings();
        }
    });
    syncViewFromGSettings();
}

QString AppCategoryPlugin::name()
{
    return QStringLiteral("AppCategoryPlugin");
}

QList<QAction *> AppCategoryPlugin::actions()
{
    return m_actions;
}

QAbstractItemModel *AppCategoryPlugin::dataModel()
{
    return m_dataModel;
}

AppListPluginGroup::Group AppCategoryPlugin::group()
{
    return AppListPluginGroup::Display;
}

QString AppCategoryPlugin::title()
{
    return m_title;
}

void AppCategoryPlugin::setTitle(const QString &title)
{
    if (title == m_title) {
        return;
    }

    m_title = title;
    Q_EMIT titleChanged();
}

LabelBottle *AppCategoryPlugin::labelBottle()
{
    updateLabelBottle();
    return m_labelBottle;
}

void AppCategoryPlugin::updateLabelBottle()
{
    QList<LabelItem> labels;
    if (m_recentlyModel->rowCount() > 0) {
        if (m_categoryModel->mode() != AppCategoryModel::All) {
            labels << LabelItem(tr("Recently Installed"), "document-open-recent-symbolic", LabelItem::Icon);
        }
    }

    QHash<QString, int> groups;
    int rowCount = m_categoryModel->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QString group = m_categoryModel->index(row, 0, QModelIndex()).data(DataEntity::Group).toString();
        if (groups.contains(group)) {
            continue;
        }

        groups.insert(group, 0);
        labels.append(LabelItem(group, group));
    }

    m_labelBottle->setLabels(labels);
    if (m_categoryModel->mode() == AppCategoryModel::All) {
        m_labelBottle->setColumn(0);
    } else {
        m_labelBottle->setColumn(m_categoryModel->mode() == AppCategoryModel::FirstLatter ? 5 : 2);
    }
}

void AppCategoryPlugin::switchTargetAction(AppCategoryModel::Mode mode,const QString &modeKey)
{
    m_categoryModel->setMode(mode);
    m_recentlyModel->setMode(mode);
    for (const QString &key : m_modeToAction.keys()) {
        m_modeToAction[key]->setChecked(key == modeKey);
    }
    setTitle(m_modeToAction[modeKey]->text());
    updateLabelBottle();
    m_labelBottle->setVisible(modeKey != "all");
}

void AppCategoryPlugin::syncViewFromGSettings()
{
    const QString modeKey = MenuSetting::instance()->get(APP_LIST_DISPLAY_MODE).toString();
    if (m_mode != modeKey) {
        if (m_modeToAction.contains(modeKey)) {
            m_mode = modeKey;
        } else {
            m_mode = "all";
        }
        AppCategoryModel::Mode mode;
        if (m_mode == "all") {
            mode = AppCategoryModel::All;
        } else if (m_mode == "letter") {
            mode = AppCategoryModel::FirstLatter;
        } else {
            mode = AppCategoryModel::Category;
        }
        switchTo(mode, m_mode, false);
    }
}

void AppCategoryPlugin::syncViewToGSettings(const QString &mode)
{
    if (MenuSetting::instance()->get(APP_LIST_DISPLAY_MODE).toString() != mode) {
        m_mode = mode;
        MenuSetting::instance()->set(APP_LIST_DISPLAY_MODE, mode);
    }
}

void AppCategoryPlugin::switchTo(AppCategoryModel::Mode mode, const QString& modeKey, bool isUserClick)
{
    switchTargetAction(mode, modeKey);
    syncViewToGSettings(modeKey);
    if (isUserClick) {
        QMap<QString, QVariant> map;
        map.insert(QStringLiteral("viewName"), modeKey);
        EventTrack::instance()->sendClickEvent("switch_app_view", "AppView", map);
    }
}

} // UkuiMenu
