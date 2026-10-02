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

#ifndef INPUTMETHODCONFIGTOOL_H
#define INPUTMETHODCONFIGTOOL_H

#include <QObject>

#include "configmanager.h"
#include "fcitxdbusprovider.h"
#include "inputmethodconfigtoolview.h"
#include "inputmethodlistmodel.h"
#include "layoutmanager.h"
#include "menumanager.h"
#include "searchinputmethodmodel.h"
#include "shortcutcontroller.h"

class InputMethodConfigTool : public QWidget
{
  Q_OBJECT

public:
  InputMethodConfigTool(QWidget *parent = nullptr);
  ~InputMethodConfigTool() override = default;

public:
  void showWindow();
  const QString currentInputMethodName();
  bool isFcitxServiceExists();

private:
  void init();
  void destroy(); // destroy all data models and views, except the dbus provider
  void connectDbusSignals();
  void connectImModelSignals();
  void connectViewSignals();

  void makeParentWindowModality();

private:
  FcitxDbusProvider *m_dbus = nullptr;
  std::unique_ptr<InputMethodListModel> m_imModel = nullptr;
  std::unique_ptr<SearchDataModel> m_searchDataModel = nullptr;
  std::unique_ptr<SearchInputMethodModel> m_searchImModel = nullptr;
  std::unique_ptr<ConfigManager> m_configManager = nullptr;
  std::unique_ptr<MenuManager> m_menuManager = nullptr;
  std::unique_ptr<LayoutManager> m_layout = nullptr;
  std::unique_ptr<ShortcutController> m_shortcutController = nullptr;
  std::unique_ptr<InputMethodConfigToolView> m_view = nullptr;
  QString m_firstInputMethodName = "";

signals:
  void firstInputMethodChanged(const QString& firstImName);
  void fcitxServiceStatusChanged(bool status);
};

#endif
