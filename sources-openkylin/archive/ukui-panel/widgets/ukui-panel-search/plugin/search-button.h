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

#ifndef UKUI_PANEL_START_MENU_BUTTON_H
#define UKUI_PANEL_START_MENU_BUTTON_H

#include <QObject>
#include <QQuickItem>

class SearchButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool disableMouseLeftButton READ disableMouseLeftButton NOTIFY disableMouseLeftButtonChanged)
    Q_PROPERTY(bool disableMouseRightButton READ disableMouseRightButton NOTIFY disableMouseRightButtonChanged)
public:
    explicit SearchButton(QObject *parent=nullptr);
    ~SearchButton() override;

    Q_INVOKABLE void openUkuiSearch(int x, int y, int width, int height);
    Q_INVOKABLE void openSearchSetting();
    Q_INVOKABLE void openContextMenu(QQuickItem *item);

    bool disableMouseLeftButton() const;
    bool disableMouseRightButton() const;

Q_SIGNALS:
    void disableMouseLeftButtonChanged();
    void disableMouseRightButtonChanged();

public Q_SLOTS:
    void onFunctionControlChanged(const QString& name, bool exit = false);

private:
    void initFunctionControl();

    bool m_disableMouseLeftButton = false;
    bool m_disableMouseRightButton = false;
};

#endif //UKUI_PANEL_START_MENU_BUTTON_H
