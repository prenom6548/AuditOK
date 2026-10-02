/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#ifndef INFOCOLLECTIONBASE_H
#define INFOCOLLECTIONBASE_H

#include <QObject>
#include <QObject>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QProcess>

class InfoCollectionBase:public QObject
{
    Q_OBJECT
public:
    InfoCollectionBase(QObject *parent=nullptr);
protected:
    /**
     * @brief 通过Popen命令获得多条结果
     *
     * @param shellCmd shell命令
     * @param result 查询结果
     */
    virtual void getOnePopenResult(const QString &shellCmd, QString &result);
    /**
     * @brief 通过Popen命令获得一条结果
     *
     * @param shellCmd shell命令
     * @param result 查询结果
     */
    virtual void getPopenResult(const QString &shellCmd, QList<QString> &result);
    /**
     * @brief 最大字符串长度
     *
     */
    static constexpr int32_t s_maxStrLen = 1024;
    /**
     * @brief 分隔符
     *
     */
    static constexpr const char* s_separator = "; ";
private:
};

#endif // INFOCOLLECTIONBASE_H
