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

#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QQmlContext>
#include <QQuickItem>

class MenuManager : public QObject {
  Q_OBJECT
public:
  explicit MenuManager(QObject *parent = nullptr);
  ~MenuManager() override = default;
  Q_INVOKABLE void setMenu(QQuickItem *item, int itemIndex,
                           const QStringList &disableActions);
  Q_INVOKABLE void showMenu();

  Q_INVOKABLE void hideMenu();

signals:
  void menuOrButtonClicked(const QString &message);
  void menuItemClicked(const QString &message, int index);
  void menuClosed();

private slots:
  void onMenuItemTriggered() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
      emit menuItemClicked(action->data().toString(), m_itemIndex);
    }
  }
  void resetMenuActionState();

private:
  std::unique_ptr<QMenu> m_menu = nullptr;
  int m_itemIndex = -1;
  QPoint m_menuPos;
};

#endif // MENUMANAGER_H
