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
 */

#ifndef UKUI_MENU_FAVORITES_CONFIG_H
#define UKUI_MENU_FAVORITES_CONFIG_H

#include <QObject>
#include <QVariant>
#include <QJsonObject>

static const QString APP_ID_SCHEME = "app://";
static const QString FILE_ID_SCHEME = "file://";
static const QString FOLDER_ID_SCHEME = "folder://";

namespace UkuiMenu {

class FavoritesConfig : public QObject
{
    Q_OBJECT
public:
    static FavoritesConfig &instance();
    void sync(const QStringList &list);
    QStringList getConfig();

private:
    explicit FavoritesConfig(QObject *parent = nullptr);
    void initConfig();
    QStringList updateInvalidUrlList(QStringList &list);

    static QString s_favoritesConfigFile;
    QStringList m_configList;
};

} // UkuiMenu

#endif //UKUI_MENU_FAVORITES_CONFIG_H
