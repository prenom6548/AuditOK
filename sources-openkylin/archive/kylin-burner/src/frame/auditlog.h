/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef AUDITLOG_H
#define AUDITLOG_H

#include <QFile>
#include <QDataStream>
#include <QTextStream>

class AuditLog
{
    friend class AuditLogTest;
public:
    AuditLog();
    ~AuditLog();
    void appendFile(const QString& filePath);
    void appendFiles(const QStringList& fileLists);
    void outputResult(const QString& recorderName, const QString& discType, const QString& operation, int burnResult);//记录刻录结果
    void outputCopyResult(const QString& mountPoint, const QString& destRercorderName,
                          QString destDiscType, const QString& operation, bool result);
    void ouputCreateResult(const QString& recorderName, const QString& targetFile, int result);
private:
    void initInfo();
    bool createAuditLog(const QString &filePath);
    QString getFileInfo(const QString& file);
    void getFilesFromMountpoint(const QString& mountPoint);
    QString stringSize(unsigned long);

private:
    QFile mLogFile;
    QString mUserName;
    QStringList mFilesList;
    QTextStream mWriteStream;
};


/* 返回上一级目录？
*/
#endif // AUDITLOG_H
