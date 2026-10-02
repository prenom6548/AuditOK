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

#ifndef INPUTMETHODCONFIGTOOLVIEW_H
#define INPUTMETHODCONFIGTOOLVIEW_H

#include <functional>

#include <QQuickView>
#include <window-helper.h>

#include "configmanager.h"
#include "inputmethodlistmodel.h"
#include "layoutmanager.h"
#include "menumanager.h"
#include "searchinputmethodmodel.h"
#include "shortcutcontroller.h"

class InputMethodConfigToolView : public QQuickView
{
  Q_OBJECT

public:
  InputMethodConfigToolView(
      InputMethodListModel *imModel, MenuManager *menuManager,
      ConfigManager *configManager, LayoutManager *layoutManager,
      SearchDataModel *searchDataModel, SearchInputMethodModel *SearchImModel, ShortcutController *shortcutController);
  ~InputMethodConfigToolView() override = default;

private:
  void makeViewBlur();
  bool event(QEvent *event) override;
private:
  std::unique_ptr<UkuiQuick::WindowProxy> m_windowProxy = nullptr;

signals:
    void activeStateChanged(bool state);
};

#endif // INPUT_METHOD_SETTINGS_WINDOW_H
