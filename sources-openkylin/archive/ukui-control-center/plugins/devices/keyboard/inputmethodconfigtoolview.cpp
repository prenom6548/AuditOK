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

#include "inputmethodconfigtoolview.h"

#include <QAction>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDebug>
#include <QQmlContext>
#include <QQuickItem>

InputMethodConfigToolView::InputMethodConfigToolView(InputMethodListModel *imModel, MenuManager *menuManager,
    ConfigManager *configManager, LayoutManager *layoutManager,
    SearchDataModel *searchDataModel, SearchInputMethodModel *SearchImModel, ShortcutController *shortcutController)
{
  setColor(Qt::transparent);
  setMinimumSize(QSize(960, 620));
  setMaximumSize(QSize(960, 620));

  rootContext()->setContextProperty("imModel", imModel);
  rootContext()->setContextProperty("menuManager", menuManager);
  rootContext()->setContextProperty("configManager", configManager);
  rootContext()->setContextProperty("searchDataModel", searchDataModel);
  rootContext()->setContextProperty("searchImModel", SearchImModel);
  rootContext()->setContextProperty("layoutManager", layoutManager);
  rootContext()->setContextProperty("shortcutController", shortcutController);
  SearchImModel->setSourceModel(searchDataModel);
  rootContext()->setContextProperty("Window", this);
  setSource(QUrl("qrc:/qml/InputMethodConfigTool.qml"));
  makeViewBlur();
}

void InputMethodConfigToolView::makeViewBlur()
{
  m_windowProxy.reset(
      new UkuiQuick::WindowProxy(this, UkuiQuick::WindowProxy::RemoveTitleBar));
  m_windowProxy->setBlurRegion(true);
}

bool InputMethodConfigToolView::event(QEvent *event)
{
  switch (event->type()) {
  case QEvent::FocusIn:
      if (isActive()) {
          emit activeStateChanged(true);
      }
      break;
  case QEvent::FocusOut:
      emit activeStateChanged(false);
      break;
  default:
      break;
  }
  return QQuickView::event(event);
}
