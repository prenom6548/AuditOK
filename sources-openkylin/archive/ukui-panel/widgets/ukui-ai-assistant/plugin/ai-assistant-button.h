/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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

namespace UkuiSearch {
    class ApplicationInfo;
}

class AiAssistantButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool disableMouseLeftButton READ disableMouseLeftButton NOTIFY disableMouseLeftButtonChanged)

public:
    explicit AiAssistantButton(QObject *parent=nullptr);
    ~AiAssistantButton() override;
    Q_INVOKABLE void pressed();

    bool available() const;
    QString icon() const;
    bool disableMouseLeftButton() const;

Q_SIGNALS:
    void availableChanged();
    void iconChanged();
    void disableMouseLeftButtonChanged();

public Q_SLOTS:
    void onFunctionControlChanged(const QString& name, bool exit = false);

private:
    void fetchIcon();
    void initFunctionControl();

    bool m_disableMouseLeftButton = false;
    UkuiSearch::ApplicationInfo *m_appInfo = nullptr;
    QString m_desktopFile;
    QString m_icon;
};

#endif //UKUI_PANEL_START_MENU_BUTTON_H
