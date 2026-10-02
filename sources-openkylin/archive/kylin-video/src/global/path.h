/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef PATH_H
#define PATH_H

#include <QString>

class Paths {

public:

    static void setAppPath(QString path);
    static QString previewPath();
    static QString appPath();
    static QString translationPath();
    static QString shortcutsPath();
    static QString qtTranslationPath();
    //! Forces to use a different path for the config files
    static void setConfigPath(QString path);
    static void setLogPath(QString path);

    //! Return the path where smplayer should save its config files
    static QString configPath();

    //! Obsolete. Just returns configPath()
    static QString iniPath();

    static QString logPath();

    static QString subtitleStyleFile();

private:
    static QString m_app_path;
    static QString m_log_path;
    static QString m_config_path;
};

#endif // PATH_H
