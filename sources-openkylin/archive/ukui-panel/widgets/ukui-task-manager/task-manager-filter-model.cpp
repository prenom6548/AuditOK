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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "task-manager-filter-model.h"
#include <QQuickWindow>
#include <QTimer>
#include "ukui-task-manager.h"
#include <window-manager.h>
#include <QGuiApplication>
namespace TaskManager {
TaskManagerFilterModel::TaskManagerFilterModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setFilterRole(UkuiTaskManager::CurrentWinIdList);
    //主屏改变时，发送事件，更新数据
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [&]() {
        if (m_sourceModel) {
            m_sourceModel->requestDataChange();
        }
    });
    setSourceModel(&UkuiTaskManager::self());
}

void TaskManagerFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    m_sourceModel = qobject_cast<UkuiTaskManager*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);
    m_iconsShowedonScreen = static_cast<TaskBarIconsShowedOn>(m_sourceModel->taskbarIconsShowedOnByControl());
    connect(sourceModel, &QAbstractItemModel::dataChanged, this, &TaskManagerFilterModel::onDataChanged);
    connect(m_sourceModel, &UkuiTaskManager::appWhiteListChanged, this, &TaskManagerFilterModel::invalidateFilter);
    connect(m_sourceModel, &UkuiTaskManager::appBlackListChanged, this, &TaskManagerFilterModel::invalidateFilter);
    connect(m_sourceModel, &UkuiTaskManager::taskbarIconsShowedOnChanged, this, [this](){
        m_iconsShowedonScreen = static_cast<TaskBarIconsShowedOn>(m_sourceModel->taskbarIconsShowedOnByControl());
        invalidateFilter();
        if (m_sourceModel) {
            m_sourceModel->requestDataChange();
        }
    });

}

QVariant TaskManagerFilterModel::data(const QModelIndex &index, int role) const
{
    if(role == UkuiTaskManager::CurrentWinIdList && m_screen) {
        return currentWinIdList(index);
    } else if(role == UkuiTaskManager::HasActiveWindow) {
        if(m_screen && mapToSource(index).data(role).toBool()) {
            return currentWinIdList(index).contains(UkuiQuick::WindowManager::currentActiveWindow());
        }
        return false;
    } else if (role == UkuiTaskManager::Actions) {
        Actions actions = mapToSource(index).data(role).value<Actions>();
        for(auto action : actions) {
            if(action->type() == Action::Type::Exit) {
                QStringList wids = currentWinIdList(index);
                action->setEnabled(!wids.isEmpty());
                action->setParam(wids);
                break;
            }
        }
        return QVariant::fromValue(actions);
    } else if (role == UkuiTaskManager::CurrentScreenWinIdList) {
        QStringList wins;
        auto windowGeometries = mapToSource(index).data(UkuiTaskManager::WindowGeometries).value<QMap<QString, QRect>>();
        for (const QString&wid: mapToSource(index).data(UkuiTaskManager::CurrentDesktopWinIdList).toStringList()) {
            if (windowOnCurrentScreen(windowGeometries.value(wid))) {
                wins.append(wid);
            }
        }
        return wins;
    }

    return sourceModel()->data(mapToSource(index), role);
}

void TaskManagerFilterModel::setOrder(const QModelIndex &index, int order)
{
    if(order >= rowCount()) {
        order = rowCount() - 1;
    }
    if(order < 0) {
        order = 0;
    }

    // 将setOrder操作放到下一次消息循环，避免wlcom上事件double free崩溃问题
    QTimer::singleShot(0, this, [this, index, order] {
        m_sourceModel->setOrder(mapToSource(index), mapToSource(this->index(order, 0)).row());
    });
}

void TaskManagerFilterModel::addQuickLauncher(const QString &desktopFile, int order)
{
    auto orderIndex = mapToSource(this->index(order, 0));
    if (!orderIndex.isValid() && order > 0 && order <= m_sourceModel->rowCount(QModelIndex())) {
        // 添加到最后一位
        m_sourceModel->setQuickLauncher(desktopFile, m_sourceModel->rowCount(QModelIndex()));
    } else {
        m_sourceModel->setQuickLauncher(desktopFile, orderIndex.row());
    }
}

QScreen *TaskManagerFilterModel::screen() const
{
    return m_screen;
}

void TaskManagerFilterModel::setScreen(QScreen *screen)
{
    m_screen = screen;
    invalidateFilter();
    m_sourceModel->requestDataChange();
}

bool TaskManagerFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    //黑白名单管控
    if (!m_sourceModel->appWhiteList().isEmpty()) {
        auto whiteList = m_sourceModel->appWhiteList();
        if (!std::any_of(whiteList.constBegin(), whiteList.constEnd(), [&](const QString& app) {
            return sourceIndex.data(UkuiTaskManager::Id).toString().endsWith(app);
        })) {
            return false;
        }
    }
    if (!m_sourceModel->appBlackList().isEmpty()) {
        auto blackList = m_sourceModel->appBlackList();
        if (std::any_of(blackList.constBegin(), blackList.constEnd(), [&](const QString& app) {
            return sourceIndex.data(UkuiTaskManager::Id).toString().endsWith(app);
        })) {
            return false;
        }
    }

    if (!sourceIndex.data(UkuiTaskManager::HasLauncher).toBool()) {
        if (sourceIndex.data(UkuiTaskManager::CurrentDesktopWinIdList).toStringList().isEmpty()) {
            return false;
        }
        if (!sourceIndex.data(UkuiTaskManager::HasLauncher).toBool()) {
            switch (m_iconsShowedonScreen) {
                case AllPanels:
                    return true;
                case PrimaryScreenPanelAndPanelWhereWindowIsOpen:
                    if (m_screen == qApp->primaryScreen() || onCurrentScreen(sourceIndex)) {
                        return true;
                    }
                break;
                case PanelWhereWindowIsOpen:
                    if (onCurrentScreen(sourceIndex)) {
                        return true;
                    }
                break;
                default:
                    return false;
            }
            return false;
        }
    }

    return true;
}

bool TaskManagerFilterModel::openMenu(const bool &showDisabledActions, const TaskManager::Actions &actions, QQuickItem *item)
{
    if (actions.isEmpty()) {
        return false;
    }

    if (m_contextMenu) {
        m_contextMenu->close();
    }

    auto *menu = new QMenu;
    menu->setAttribute(Qt::WA_DeleteOnClose);

    for(const TaskManager::Action* action : actions) {
        if(action->enabled() || showDisabledActions) {
            menu->addAction(action->internalAction());
        }
        if (action->type() == TaskManager::Action::NewInstance) {
            menu->insertSeparator(action->internalAction());
        }
    }

    connect(menu, &QMenu::aboutToHide, this, [this] {
            Q_EMIT menuAboutToHide();
    });
    m_contextMenu = menu;
    m_contextMenu->winId();
    if(m_contextMenu->windowHandle()) {
        m_contextMenu->windowHandle()->setTransientParent(item->window());
    }
    m_contextMenu->popup(QCursor::pos());
    return true;
}

TaskManager::Actions TaskManagerFilterModel::menuActions(int row)
{
    if (!m_sourceModel || row < 0 || row >= rowCount()) {
        return {};
    }

    const QModelIndex proxyIndex = index(row, 0);
    if (!proxyIndex.isValid()) {
        return {};
    }

    Actions actions = m_sourceModel->menuActions(mapToSource(proxyIndex));
    for (auto action : actions) {
        if (action->type() == Action::Type::Exit) {
            QStringList wids = currentWinIdList(proxyIndex);
            action->setEnabled(!wids.isEmpty());
            action->setParam(wids);
            break;
        }
    }

    return actions;
}

QStringList TaskManagerFilterModel::currentWinIdList(const QModelIndex &index) const
{
    if (m_iconsShowedonScreen == AllPanels) {
        return mapToSource(index).data(UkuiTaskManager::CurrentDesktopWinIdList).toStringList();
    }
    QStringList wins;
    auto windowGeometries = mapToSource(index).data(UkuiTaskManager::WindowGeometries).value<QMap<QString, QRect>>();
    for (const QString&wid: mapToSource(index).data(UkuiTaskManager::CurrentDesktopWinIdList).toStringList()) {
        switch (m_iconsShowedonScreen) {
            case PrimaryScreenPanelAndPanelWhereWindowIsOpen:
                if (windowOnCurrentScreen(windowGeometries.value(wid)) || m_screen == qApp->primaryScreen()) {
                    wins.append(wid);
                }
                break;
            case PanelWhereWindowIsOpen:
                if (windowOnCurrentScreen(windowGeometries.value(wid))) {
                    wins.append(wid);
                }
                break;
            default:;
        }
    }
    return wins;
}

void TaskManagerFilterModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                           const QVector<int> &roles)
{
    Q_EMIT dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
    if(roles.contains(UkuiTaskManager::CurrentDesktopWinIdList) || roles.contains(UkuiTaskManager::WindowGeometries)) {
        Q_EMIT dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), QVector<int>{UkuiTaskManager::HasActiveWindow, UkuiTaskManager::CurrentScreenWinIdList});
    }
}

bool TaskManagerFilterModel::onCurrentScreen(const QModelIndex& index) const
{
    //item在当前desktop上的窗口中是否包含一个在当前屏幕上的窗口
    auto windowGeometries = index.data(UkuiTaskManager::WindowGeometries).value<QMap<QString,QRect>>();
    for(const QString &wid : index.data(UkuiTaskManager::CurrentDesktopWinIdList).toStringList()) {
        if(windowOnCurrentScreen(windowGeometries.value(wid))) {
            return true;
        }
    }
    return false;
}

bool TaskManagerFilterModel::windowOnCurrentScreen(const QRect &rect) const
{
    if(!QGuiApplication::screens().contains(m_screen)) {
        return false;
    }
    auto intersectedRect = m_screen->geometry().intersected(rect);
    if(intersectedRect.isEmpty()) {
        return false;
    }
    for(const auto &screen : qGuiApp->screens()) {
        QRect r = screen->geometry().intersected(rect);
        if(r.isEmpty()) {
            continue;
        }
        if(r.width() * r.height() > intersectedRect.width() * intersectedRect.height() && screen != m_screen) {
            return false;
        }
    }
    return true;
}

void TaskManagerFilterModel::launch(const QModelIndex &index, QQuickItem *item)
{
    m_sourceModel->launch(mapToSource(index), item);
}
}
