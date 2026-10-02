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

#ifndef AISEARCHTASK_H
#define AISEARCHTASK_H

#include <QObject>
#include <QJsonDocument>
#include <QVariant>
#include <atomic>
#include <business/data-management/datamanagement.h>
#include <memory>
#include <file/fileinfo.h>
namespace UKUI {
namespace IDM {
class FileInfo;
}
}

class AISearchTask : public QObject
{
    Q_OBJECT
public:
    explicit AISearchTask(const QString &description, QObject *parent = nullptr);

    /*!
     * \brief setPharseConditions
     * 是否对输入的description进行语义解析转为条件之后再进行搜索
     * \param newPharseConditions
     */
    void setPharseConditions(bool newPharseConditions);

    /*!
     * \brief setFilterConditionDescription
     * 设置用于过滤结果的条件配置，只有在pharseConditions为false时才有效果
     * \param conditionDescription
     */
    void setFilterConditionDescription(const QVariant &conditionDescription);

    void setSpaceId(int spaceId);

Q_SIGNALS:
    void finisheds(bool successed, const QList<QString> &results, const QJsonDocument &conditions, const QString &errorMsg);
    void finished(bool successed, const QList<QString> &results, const QJsonDocument &conditions, const QString &errorMsg);
    void searchcontentload(QString uri, QString modified, QString filetype,QString size);

public Q_SLOTS:
    void cancel();
    void doQuery();
    void cancelWithSpaceId(int spaceId);

private:
    std::atomic_bool cancelled;
    bool pharseConditions = false;
    QString m_description;
    QVariant m_conditionDescription;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> resultInfos;

    int spaceId = -1;
};

#endif // AISEARCHTASK_H
