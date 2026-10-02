/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef SHREDQTHREAD_H
#define SHREDQTHREAD_H

#include "./filewipe.h"
#include <QThread>
#include <QObject>
#include <QMap>

#define emit Q_EMIT
#define foreach Q_FOREACH
class ShredQThread : public QObject
{
    Q_OBJECT
public:
    ShredQThread(QStringList strFilePath);
    ~ShredQThread();

public Q_SLOTS:
    void run();

Q_SIGNALS:
    void success();
    // void failed(QStringList failFileName, QStringList successFileName);
    void failed(QStringList failFileName, QStringList successFileName, QStringList allFailFiles);

private:
    QStringList m_strFileNames;
    QStringList m_failFileName = {};
    QStringList m_successFileName = {};
    QStringList m_fileList = {}; //需要粉碎的文件夹及文件的list

    QMap<QString, bool> m_dirMap;
    QMap<QString, QString> m_dirRelation;

    QStringList listOfShredFiles(QString targetFile); //找到需要粉碎的文件列表
    void findRmDir(QString dirPath);                  //查找需要删除的文件夹
    bool removeDir(QString dir);                      //执行文件夹删除功能
    void removeFile(QString file);                      //执行文件删除功能
    QStringList m_allFailFiles = {};                  //所有失败的文件
    QStringList m_allRmDirs = {};                     //所有失败的文件夹
    QStringList m_allRemainFailDirs = {};             //所有删除失败的主文件夹
};

#endif // SHREDQTHREAD_H
