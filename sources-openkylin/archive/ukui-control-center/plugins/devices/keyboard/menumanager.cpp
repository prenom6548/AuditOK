/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "menumanager.h"

MenuManager::MenuManager(QObject *parent) : QObject(parent) {

  m_menu = std::make_unique<QMenu>();
  m_menu->setAccessibleName(tr("Option"));
  QAction *actionMoveUp = m_menu->addAction(tr("Move Up"));
  QAction *actionMoveDown = m_menu->addAction(tr("Move Down"));
  QAction *actionSettings = m_menu->addAction(tr("Settings"));
  QAction *actionDelete = m_menu->addAction(tr("Delete"));

  actionMoveUp->setData("moveup");
  actionMoveDown->setData("movedown");
  actionDelete->setData("delete");
  actionSettings->setData("settings");

  connect(actionMoveUp, &QAction::triggered, this,
          &MenuManager::onMenuItemTriggered);
  connect(actionMoveDown, &QAction::triggered, this,
          &MenuManager::onMenuItemTriggered);
  connect(actionDelete, &QAction::triggered, this,
          &MenuManager::onMenuItemTriggered);
  connect(actionSettings, &QAction::triggered, this,
          &MenuManager::onMenuItemTriggered);
  connect(m_menu.get(), &QMenu::aboutToHide, this,
          &MenuManager::resetMenuActionState);
  connect(m_menu.get(), &QMenu::aboutToHide, this, &MenuManager::menuClosed);
}

void MenuManager::setMenu(QQuickItem *item, int itemIndex,
                          const QStringList &disableActions) {
  if (!m_menu) {
    return;
  }
  if (item) {
    m_menuPos = item->mapToGlobal(QPoint(1, item->height() + 1)).toPoint();
    for (auto &disable : disableActions) {
      QList<QAction *> actions = m_menu->actions();
      for (QAction *action : actions) {
        if (action->data() == disable) {
          action->setEnabled(false);
          break;
        }
      }
    }
    m_itemIndex = itemIndex;
  }
}

void MenuManager::showMenu() {
  if (!m_menu) {
    return;
  }
  m_menu->popup(m_menuPos);
}

void MenuManager::hideMenu() {
  if (!m_menu) {
    return;
  }
  m_menu->hide();
}

void MenuManager::resetMenuActionState() {
  if (!m_menu) {
    return;
  }
  QList<QAction *> actions = m_menu->actions();
  for (QAction *action : actions) {
    action->setEnabled(true);
  }
}
