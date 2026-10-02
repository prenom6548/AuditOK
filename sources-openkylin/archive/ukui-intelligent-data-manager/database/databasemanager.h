/*
 * UKUI Intelligent Data Manager
 *
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QMap>

class DatabaseManagerPrivate;

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(const QString &dbname = "ukui-idm.db", QObject *parent = nullptr);
    ~DatabaseManager();

    /*!
     * \brief getAllSpaceNames
     * \param errorMessage
     * \return all active space
     * \note removed spaces will not in this list, but still has a record and unique id in database.
     */
    QStringList getAllSpaceNames(QString *errorMessage = nullptr);

    bool createSpace(const QString &spaceName, const QStringList &conditions, QString *errorMessage = nullptr, QString *resultName = nullptr);
    bool renameSpace(const QString &currentSpaceName, const QString &newSpaceName, QString *errorMessage = nullptr);
    bool removeSpace(const QString &spaceName, QString *errorMessage = nullptr);
    QStringList getSpaceCondition(const QString &spaceName, QString *errorMessage = nullptr);
    bool changeSpaceConditions(const QString &spaceName, const QStringList &conditions, QString *errorMessage = nullptr);

    bool addFileToSpace(const QString &filePath, const QString &spaceName, bool isUserAdded = false, QString *errorMessage = nullptr); // deprecated
    bool addFileToSpace(const QString &filePath, int spaceId, bool isUserAdded = false, QString *errorMessage = nullptr);
    bool removeFileFromSpace(const QString &filePath, const QString &spaceName, bool isUserRemove = false, QString *errorMessage = nullptr); // deprecated
    bool removeFileFromSpace(const QString &filePath, int spaceId, bool isUserRemove = false, QString *errorMessage = nullptr);
    bool removeFileFromAllSpace(const QString &filePath, QString *errorMessage = nullptr);

    QStringList getAllFilesFromSpaces(bool isUserAdded = false, QString *errorMessage = nullptr);

    // todo: a batch function for improve database performance.
    // 当前测试环境中4000数据同时加入2个表格耗时已经达到10s级别，需要优化。
    // 文件表格的批处理可能需要借助FileInfoManager实现，对于已经存在的文件和新文件做不同的处理

    /*!
     * \brief getAllFilesWithSpaceIds
     * \param isUserAdded
     * \param errorMessage
     * \return all files in all spaces with space id info.
     */
    QList<QPair<QString, QList<int>>> getAllFilesWithSpaceIds(bool isUserAdded = false, QString *errorMessage = nullptr);

    /*!
     * \brief getAllFilesFromSpacesBySpaceIds
     * \param isUserAdded
     * \param errorMessage
     * \return a map with spaces and files in those spaces, the map key is space id in database.
     * \sa getSpaceIdByName()
     */
    QMap<int, QStringList> getAllFilesFromSpacesBySpaceIds(bool isUserAdded = false, QString *errorMessage = nullptr);

    int getSpaceIdByName(const QString &spaceName, QString *errorMessage = nullptr);
    int getFileIdByPath(const QString &filePath, bool isUserAdded, QString *errorMessage = nullptr);
signals:

private:
    void setErrorString(const QString &errorMessage, QString *targetString);

    DatabaseManagerPrivate *priv;
};

#endif // DATABASEMANAGER_H
