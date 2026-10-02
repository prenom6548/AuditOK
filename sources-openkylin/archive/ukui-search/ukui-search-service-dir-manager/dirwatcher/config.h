/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */
#ifndef CONFIG_H
#define CONFIG_H
#include <QString>
#include <QSettings>
#include <QVector>
#include <QJsonObject>
#include "search-dir.h"

class Config : public QObject
{
    Q_OBJECT
public:
    static Config *self();
    void addDir(const SearchDir& dir);
    QStringList removeDir(const QString &path);

    void setExtraDirs(const QStringList extraDirs);

    int addBlockDirOfUser(const QString& dir);
    void removeBlockDirOfUser(const QString& dir);

    /**
     * @brief 处理老版本数据，需要调用
     */
    void processCompatibleCache();
    QStringList searchDirs();
    QStringList blackDirs();
    QStringList globalBlackList();
    QStringList blockDirsForUser();
    QStringList extraDirs();
    bool showInUkcc(const QString& dir);


private:
    explicit Config(QObject *parent = nullptr);
    void checkSearchDir();
    bool isCompatibilityMode();
    void save(const QJsonObject &jsonDocData);
    void initSettingsData(QJsonObject &data);
    void initDefaultCompatibleConfig();
    QStringList filterSubDirs(QStringList& paths);

    QJsonObject m_settingsData;
    bool m_compatibilityMode = false;
    QVector<SearchDir> m_compatibleCache;
};

#endif // CONFIG_H
