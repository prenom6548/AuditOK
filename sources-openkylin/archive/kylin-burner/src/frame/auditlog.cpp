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

#include "auditlog.h"
#include "logger.h"
#include "src/po/translation.h"
#include <glib.h>

#include <QDir>
#include <QObject>
#include <QFileInfo>
#include <QDateTime>
#include <QMimeDatabase>

#if KAIMING_BUILD
    const QString auditLogFile = "/var/log/kaiming/kylin-burner/os_audit.log";
#else
    const QString auditLogFile = "/var/log/kylin-burner/os_audit.log";
#endif

AuditLog::AuditLog(){
    initInfo();
}

/** 针对光盘进行的操作以及结果记录到审计日志中
 *  适用于光盘擦除、刻录数据、刻录镜像操作
 *  注：调用该函数前需要手动调用appendFile()或者appendFiles()传递源文件
 *
 *  @recorderName 光驱型号        @discType 光盘类型(eg DVD+R)
 *  @operation 操作(如擦除光盘)    @burnResult 操作的结果(1成功，0失败)
*/
void AuditLog::outputResult(const QString& recorderName, const QString& discType, const QString& operation, int burnResult){

    if(!mLogFile.isOpen()){
        Logger::instance()->warn(QString("AuditLog::outputResult : open %1 fail").arg(mLogFile.fileName()));
        return;
    }
#ifndef UNIT_TEST
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    //写日志
    mWriteStream<<_("Time")        << "=" << currentTime                <<", "
                <<_("User")        << "=" << mUserName                  <<", "
                <<_("RecorderType")<< "=" << recorderName.trimmed()     <<", "
                <<_("DiscProperty")<< "=" << discType                   <<", "
                <<_("FileName")    << "=";

    if(!mFilesList.isEmpty()){

        for(auto& tmpFile : mFilesList){
            mWriteStream << getFileInfo(tmpFile);
        }

    }else{
        mWriteStream<<_("Current operation donot need file");       //擦除操作不需要考虑文件
    }

    mWriteStream<<", ";
    mWriteStream<<_("Operation")  << "=" << operation                  <<", "
                <<_("Result")     << "=" << (burnResult? _("success"):_("fail"))
                <<"\n";

    mWriteStream.flush();
    mFilesList.clear();
#endif
}

/** 拷贝光盘或提取镜像的信息，记录到审计日志
*/
void AuditLog::outputCopyResult(const QString& mountPoint, const QString &destRecorder,
                                QString destDiscType, const QString &operation, bool result){
#ifndef UNIT_TEST
    if(!mLogFile.isOpen()){
        Logger::instance()->warn(QString("AuditLog::outputCopyResult : open %1 fail").arg(mLogFile.fileName()));
        return;
    }

    if(destRecorder.isEmpty()){
        Logger::instance()->warn(QString("AuditLog::outputCopyResult : destRecorder is null"));
        return;
    }

    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    mWriteStream<<_("Time")           <<"="<<currentTime              <<", "
                <<_("User")           <<"="<<mUserName                <<", "
                <<_("RecorderType")   <<"="<<destRecorder.trimmed()   <<", "
                <<_("DiscProperty")   <<"=";

    //如果是提取镜像操作，不需要写光盘类型(因为没有目标光盘)
    if(destDiscType.isEmpty())
        destDiscType = _("Donot need disc");
    mWriteStream<<destDiscType<<", ";

    //提取镜像时，输出源光盘的挂载点目录下的文件
    getFilesFromMountpoint(mountPoint);
    if(!mFilesList.isEmpty()){
        mWriteStream<<_("FileName")  <<"=";

        for(auto& tmpFile : mFilesList){
            mWriteStream<<getFileInfo(tmpFile);
        }

        mWriteStream<<", ";
    }

    mWriteStream<<_("Operation")      <<"="<<operation                <<", "
                <<_("Result")         <<"="<<(result? _("success"):_("fail"))
                <<"\n";
    mWriteStream.flush();
    mFilesList.clear();
#endif
}

/** 不需要刻录机时的创建镜像操作，操作和结果记录到审计日志
 *  注：调用该函数前需要手动调用appendFile()或者appendFiles()传递源文件
*/
void AuditLog::ouputCreateResult(const QString &recorderName, const QString &targetFile, int result){
#ifndef UNIT_TEST
    if(!mLogFile.isOpen()){
        Logger::instance()->warn(QString("AuditLog::ouputCreateResult : open %1 fail").arg(mLogFile.fileName()));
        return;
    }

    if(mFilesList.isEmpty()){                            //需要文件才能创建iso文件
        Logger::instance()->warn(QString("AuditLog::ouputCreateResult : file list path is null"));
        return;
    }

    if(recorderName.isEmpty() || targetFile.isEmpty()) { //刻录机名和目标iso文件名
        Logger::instance()->warn(QString("AuditLog::ouputCreateResult : recorderName or targetFile is null"));
        return;
    }

    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    mWriteStream<<_("Time")           <<"="<<currentTime              <<", "
                <<_("User")           <<"="<<mUserName                <<", "
                <<_("RecorderType")   <<"="<<recorderName.trimmed()   <<", "
                <<_("DiscProperty")   <<"="<<_("Donot need disc")     <<", "
                <<_("FileName")       <<"=";                                    //源文件列表

    //源文件列表
    for(auto& tmpFile : mFilesList)
        mWriteStream<<getFileInfo(tmpFile);
    mWriteStream<<", ";

    mWriteStream<<_("ImageFile")      <<"="<<getFileInfo(targetFile)  <<", "     //镜像文件路径
                <<_("Operation")      <<"="<<_("Creating ISO")        <<", "
                <<_("Result")         <<"="<<(result? _("success"):_("fail"))
                <<"\n";
    mWriteStream.flush();
    mFilesList.clear();
#endif
}

QString AuditLog::getFileInfo(const QString &file){

    QString fileInfo;
#ifndef UNIT_TEST
    if(file.isEmpty())
        return fileInfo;

    QMimeDatabase mimeData;
    QMimeType currentFileType;
    QFileInfo currentFile(file);

    //求文件大小
    qint64 byteSize = currentFile.size();                           //计算文件大小
     QString fileSize=stringSize(byteSize);


    //获取文件类型
    currentFileType = mimeData.mimeTypeForFile(currentFile);

    //拼接文件信息
    fileInfo = currentFile.filePath();
    fileInfo += " (";
    fileInfo += currentFileType.comment();
    fileInfo += ", ";
    fileInfo += fileSize;
    fileInfo += ") ";
#endif
    return  fileInfo;
}

void AuditLog::getFilesFromMountpoint(const QString& mountPoint){
    if(mountPoint.isEmpty()){
        return;
    }

    QDir dir(mountPoint);

    if(!dir.exists()){
        return;
    }

    QStringList filesList = dir.entryList(QDir::Files|QDir::Dirs|QDir::Hidden|QDir::NoDotAndDotDot);
    for(auto& file : filesList){
        mFilesList.append(mountPoint + "/" + file);
    }
}

/** 添加待写入到光盘内的文件
*/
void AuditLog::appendFile(const QString &filePath){
    if(filePath.isEmpty()){
        Logger::instance()->warn(QString("AuditLog::appendFile : filePah is empty"));
        return;
    }

    if(filePath.contains(".cache/KylinBurner/")){
        return;
    }

    mFilesList.append(filePath);
}

void AuditLog::appendFiles(const QStringList &fileLists){
    if(fileLists.isEmpty()){
        Logger::instance()->warn(QString("AuditLog::appendFiles : fileLists is empty"));
        return;
    }

    mFilesList.append(fileLists);
}

void AuditLog::initInfo(){

#if KAIMING_BUILD
    createAuditLog(auditLogFile);
#endif

    mUserName = qgetenv("USER");
    if(mUserName.isEmpty())
        mUserName = qgetenv("USERNAME");

    mLogFile.setFileName(auditLogFile);

    if(mLogFile.open(QIODevice::WriteOnly | QIODevice::Append)) {      //只读方式打开
        mWriteStream.setDevice(&mLogFile);
    } else {
        Logger::instance()->warn(QString("AuditLog::initInfo : open fail").arg(auditLogFile));
    }
}


bool AuditLog::createAuditLog(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    if (fileInfo.exists()) {
        Logger::instance()->warn(QString("Audit log file : [%1] already exists").arg(filePath));
        return true;
    }

    QDir parentDir = fileInfo.dir();
    if (!parentDir.exists()) {
        if (!parentDir.mkpath(".")) {
            Logger::instance()->warn(QString("Failed to create directory: [%1]").arg(parentDir.path()));
            return false;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        Logger::instance()->warn(QString("Failed to create audit log file:: [%1]").arg(filePath));
        return false;
    }
    file.close();
    return true;
}

QString AuditLog::stringSize(unsigned long size)
{

    int unit;
    qreal rsize;
    QString ret;

    unit = -1;
    rsize = size * 1.0;

    while (rsize > 1024.00)
    {
        ++unit; rsize /= 1024.00;
    }

    ret = QString("%1 ").arg(rsize, 0, 'f', 1);

    switch (unit)
    {
    case 0:
        ret.append(_("KB"));
        break;
    case 1:
        ret.append(_("MB"));
        break;
    case 2:
        ret.append(_("GB"));
        break;
    default:
        ret.append(_("B"));
        break;
    }
    return ret;
}

AuditLog::~AuditLog(){
    if(mLogFile.isOpen()){
        mLogFile.flush();
        mLogFile.close();
    }
}
