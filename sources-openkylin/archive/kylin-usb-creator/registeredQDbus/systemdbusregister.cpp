/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "systemdbusregister.h"
#include <polkitqt1-authority.h>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDebug>
#include <QThread>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDBusConnectionInterface>

using namespace PolkitQt1;

SystemDbusRegister::SystemDbusRegister()
{

}

// 添加缺失的函数实现
bool SystemDbusRegister::unmountDevice(QString)
{
    // 空实现，仅用于解决链接错误
    return false;
}

//excute dd command
void SystemDbusRegister::MakeStart(QString sourcePath,QString targetPath){
    //root authorization
    //   防命令注入
    //  1、形如：mkdir '`id&>id_bak_test.txt`'中的文件夹名称
    if (sourcePath.contains(QRegularExpression(".*`.*`.*"))||targetPath.contains(QRegularExpression(".*`.*`.*")))
        return ;
    // 2、形如：$()的文件夹名称
    if (sourcePath.contains(QRegularExpression(".*\\$\\(.*\\).*"))||targetPath.contains(QRegularExpression(".*\\$\\(.*\\).*")))
        return ;
    // 3、形如：${}的文件夹名称
    if (sourcePath.contains(QRegularExpression(".*\\$\\{.*\\}.*"))||targetPath.contains(QRegularExpression(".*\\$\\{.*\\}.*")))
        return ;
    // 4、包含[;、&、|]等可以包含并执行系统命令或用于连续执行系统命令的符号
    if (sourcePath.contains(QRegularExpression("[;&|]+"))||targetPath.contains(QRegularExpression("[;&|]+")))
        return ;

    sourceFile = sourcePath;
    m_isdeviceexist=false;
    m_getmountpoint=false;
        qDebug()<<__LINE__<<__func__<<m_getmountpoint;
        Authority::Result result;
        SystemBusNameSubject subject(message().service());
        result = Authority::instance()->checkAuthorizationSync("com.kylinusbcreator.systemdbus.authoritycheck",
                                                               subject , Authority::AllowUserInteraction);
        if (result == Authority::Yes){
            //TODO: send authorization failed dbus message
            emit authorityStatus("success");
        }else{
            //there's two cases NO and Challenge
            emit authorityStatus("failed");
            return ;
        }

    env = QProcessEnvironment::systemEnvironment();
    uDiskPath = targetPath;
    QFileInfo info(sourcePath);
    sourceFileSize = info.size()/1000000;
    // if(m_formate=="true"){
    if(umountPath(uDiskPath)){
        QThread::sleep(1);
        qInfo("开始进入格式化——————————————");
        bash=new QProcess(this);
        connect(bash,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(mkfs_finished(int,QProcess::ExitStatus)));
        connect(bash, &QProcess::readyReadStandardOutput, this, [=]() {
            mkfsoutput=bash->readAllStandardOutput();
            qDebug()<<__LINE__<<__func__<<mkfsoutput;
        });
        connect(bash, &QProcess::readyReadStandardError, this, [=]() {
            mkfserror=bash->readAllStandardError();
            qDebug()<<__LINE__<<__func__<<mkfserror;
        });
        qInfo("开始进入格式化1");
        QStringList mkfsargs;
        qInfo("开始进入格式化2");
        qInfo()<<__LINE__<<__func__<<uDiskPath;
        mkfsargs<<"-F32"<<"-v"<<"-I"<<uDiskPath.toLocal8Bit();
        qInfo("开始进入格式化3------");
        bash->start("/usr/sbin/mkfs.fat",mkfsargs);
        qInfo("开始进入格式化4");
        if (bash->waitForStarted(-1)){
            qInfo("开始进入格式5");
        }else{
            emit makeFinish("make_error");
            qInfo()<<__LINE__<<__func__<<"格式化未启动";
        }
    }else{
        emit makeFinish("make_error");
        qInfo()<<__LINE__<<__func__<<"卸载失败";
    }
    qInfo("开始进入格式化6");
}

void SystemDbusRegister::mkfs_finished(int exitCode, QProcess::ExitStatus exitStatus){
    if(exitStatus==QProcess::NormalExit&& exitCode==0){
        qInfo("格式化结束");
        QThread::sleep(1);
        qInfo("格式化结束1");
        getmountpoint();
        qInfo("格式化结束2");
        if(m_getmountpoint==true){
            command_dd = new QProcess();
            //指定输出语言
            env.insert("LANGUAGE","en_US:en");
            command_dd->setProcessEnvironment(env);
            connect(command_dd,&QProcess::readyReadStandardOutput,this,&SystemDbusRegister::readBashStandardErrorInfo);
            connect(command_dd,&QProcess::readyReadStandardError,this,&SystemDbusRegister::readBashStandardErrorInfo);
            connect(command_dd,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(dd_finished(int,QProcess::ExitStatus)));
            m_isFinished = false;
            QTimer::singleShot(1*1000, this, &SystemDbusRegister::handletimeout);
            checkMd5FinishTimer = new QTimer;
            connect(checkMd5FinishTimer, &QTimer::timeout, this, &SystemDbusRegister::checkmd5finish);
            QStringList args;
            args<<"if="+sourceFile.toLocal8Bit()<<"of="+uDiskPath.toLocal8Bit()<<"status=progress"<<"bs=16M";
            command_dd->start("dd", args);
            if(command_dd->waitForStarted()){
                qInfo()<<"97command_dd->arguments()"<<command_dd->arguments();

            }
            qInfo()<<__LINE__<<__func__<<"dd开始";
            m_isStarted = true;
        }else{
            qInfo("格式化结束3");
            mountDevice(uDiskPath);
            qInfo("格式化结束4");
            QThread::sleep(1);
            command_dd = new QProcess();
            //指定输出语言
            env.insert("LANGUAGE","en_US:en");
            command_dd->setProcessEnvironment(env);
            connect(command_dd,&QProcess::readyReadStandardOutput,this,&SystemDbusRegister::readBashStandardErrorInfo);
            connect(command_dd,&QProcess::readyReadStandardError,this,&SystemDbusRegister::readBashStandardErrorInfo);
            connect(command_dd,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(dd_finished(int,QProcess::ExitStatus)));
            m_isFinished = false;
            QTimer::singleShot(1*1000, this, &SystemDbusRegister::handletimeout);
            checkMd5FinishTimer = new QTimer;
            connect(checkMd5FinishTimer, &QTimer::timeout, this, &SystemDbusRegister::checkmd5finish);
            QStringList args;
            args<<"if="+sourceFile.toLocal8Bit()<<"of="+uDiskPath.toLocal8Bit()<<"status=progress"<<"bs=16M";
            command_dd->start("dd", args);
            if(command_dd->waitForStarted()){
                qInfo()<<"97command_dd->arguments()"<<command_dd->arguments();

            }
            qInfo()<<__LINE__<<__func__<<"dd开始";
            m_isStarted = true;
        }
    } else{
        qInfo()<<__LINE__<<__func__<<"格式化失败"<<"退出码:"<<exitCode<<bash->errorString();
        m_isFinished = true;
        emit makeFinish("fail");
        qInfo()<<__LINE__<<__func__<<"发送退出";
    }
}
void SystemDbusRegister::dd_finished(int exitCode, QProcess::ExitStatus exitStatus){ 
    if(exitStatus!=QProcess::NormalExit&& exitCode!=0){
        m_isStarted = false;
        m_isFinished = true;
        qInfo("make_error");
        qInfo()<<__func__<<__LINE__<<"dd异常退出"<<command_dd->errorString();
        emit makeFinish("make_error");
    } else if (m_isStarted) {
        m_isStarted = false;
        qInfo()<<__func__<<__LINE__<<"dd正常退出";
        finishEvent();
    } else {
        m_isStarted = false;
        m_isFinished = true;
    }
}

/**
 * @brief 判断程序是否已开启
 * @param processName
 * @return bool
 */
bool SystemDbusRegister::isRunning(const QString &processName)
{
    bool bRet = false;

    QProcess ps;
    QProcess grep;
    ps.setStandardOutputProcess(&grep);

    ps.start("ps", QStringList() << "-ef");
    grep.start("grep", QStringList() << processName);

    if (grep.waitForFinished()) {
        QString result(grep.readAll());
        //qDebug() << result;
        QStringList lines = result.split("\n");
        for (const QString& line : lines) {
            if (line.contains("grep"))
                continue;
            if (line.contains(processName)){
                bRet = true;
                break;
            }
        }
    }

    grep.close();
    ps.close();

    return bRet;
}

void SystemDbusRegister::handletimeout(){
    if (m_isFinished)
        return;

    QFile file(sourceFile);
    if (file.exists()) {
        if (m_isStarted) {
            qInfo("dd running...");
            // 判断dd是否还在，不在就报错
            if (!isRunning("/usr/bin/dd "))
            {
                qInfo()<<__LINE__<<__func__<<"dd不在运行"<<m_isStarted;
                command_dd->kill();
                // emit makeFinish("make_error");
            }
        }
        QTimer::singleShot(1*1000, this, &SystemDbusRegister::handletimeout);
    } else {
        qInfo("源文件不存在，dd退出");
        command_dd->kill();
        emit makeFinish("make_error");
    }
}

//kill task process
void SystemDbusRegister::MakeExit(){
    //    qDebug()<<"exit dd process";
    //    if(!command_dd) return ;
    //    if(command_dd->Running){
    //        qDebug()<<"dd Runing";
    //        command_dd->kill();
    //        return ;
    //    }
    if(isRunning("/usr/bin/kylin-usb-creator-sysdbus")){
        qDebug()<<"exit kylin-usb-creator-sysdbus process";
        if(bash != nullptr)
        {
            bash->kill();
        }
        if(command_dd != nullptr)
        {
            command_dd->kill();
        }
        if(umount.state() == QProcess::Running)
        {
            umount.kill();
        }
        if(mount.state() == QProcess::Running)
        {
            mount.kill();
        }
        if(md5 != nullptr)
        {
            md5->kill();
        }
        exit(-1);
        qDebug()<<"exit kylin-usb-creator-sysdbus process quit";
        return ;
    }
    return ;
}

void SystemDbusRegister::readBashStandardErrorInfo()
{
    QByteArray cmdout = command_dd->readAllStandardError();
    cmdout.trimmed();
    if(!cmdout.isEmpty() && cmdout != "\r" && cmdout != "\n"){
        QString str = cmdout;
        qInfo()<<"dd写入.."<<str;
        str = str.replace(" ","");
        //        if(str =="" || str.contains("[sudo]")) {return;}
        str = str.replace("\r","");
        QStringList bytes2 = str.split("bytes");
        QString size_progress = bytes2.first();
        bool ok = false;
        qulonglong progress_num = size_progress.toDouble(&ok)/1048576;
        int mission_percent = progress_num*100/sourceFileSize;
        qInfo()<<"mission_percent "<<mission_percent;

        if(!mission_percent){
            qInfo()<<"mission_percent is 0"<<str;
            return ;
        }
        QString ii=QString::number(mission_percent);
        if(ii.isEmpty()){
            return;
        }
        //send mission percent debus message every output
        emit workingProgress(mission_percent);
    }else{
        qInfo()<<"cmdout is empty,return";
        return;
    }
}

void SystemDbusRegister::finishEvent(){
    qInfo()<<"dd结束";
    QTimer *diskRefreshDelay = new QTimer;
    connect(diskRefreshDelay,&QTimer::timeout,[=]{
        diskRefreshDelay->stop();
        isMakingSucess();
        qInfo()<<"ISMAKEING SCCESS";
        if(m_isdeviceexist==true){
            //send production success dbus message
            qInfo()<<"make success开始同步";
            sync=new QProcess();
            connect(sync,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(syncreadStandardOutput(int,QProcess::ExitStatus)));
            sync->start("sync");
            sync->waitForStarted();
            emit makeFinish("sync");
        }else{
            //send production failure dbus message
            qInfo()<<"多分区U盘"<<"make failed";
            m_isFinished = true;
            emit makeFinish("fail");
        }
    });
    diskRefreshDelay->start(2000);
}
void SystemDbusRegister::syncreadStandardOutput(int exitCode, QProcess::ExitStatus exitStatus){
    QString syncstr;
    QString syncerr;
    if(QProcess::NormalExit==exitStatus&&exitCode==0){
        qInfo()<<"同步正常结束";
        qInfo()<<"sync success!";
        emit makeFinish("unsync");

        qInfo()<<sync->arguments();
        QThread::sleep(1);
        umountPath(uDiskPath);
        QThread::sleep(1);
        qDebug()<<__LINE__<<__func__<<"进入自动挂载点校验";
        mountDevice(uDiskPath);
        QThread::sleep(1);
        checkmd5();
        m_isFinished = true;
    }else{
        qWarning()<<"An unknown error occurred! Mount process exit code:"<<sync->exitCode();
        syncstr=sync->errorString();
        syncerr=sync->readAllStandardError();
        qInfo()<<__LINE__<<__func__<<"挂载输出"<<syncerr;
        qInfo()<<__LINE__<<__func__<<"挂载err输出"<<syncstr;
        emit makeFinish("fail");
        emit makeFinish("unsync");
        qInfo()<<"同步失败";
    }

}
bool SystemDbusRegister::mountDevice(QString target){
    QString mountstr;
    QString mounterr;
    qInfo()<<"start mounting disk:"<<target;
    QDir dir;
    mountpath = "/mnt/kylinusbcreator/";
    QDir::setCurrent("/mnt/");
    dir.mkdir("kylinusbcreator");
    QDir::setCurrent(QDir::rootPath());
    QStringList stringlist;
    stringlist << target << mountpath << "-v";
    qInfo() << target << " " << mountpath << "-v";
    mount.start("mount",stringlist);
    mount.waitForStarted();
    mount.waitForFinished(-1);
    if(QProcess::NormalExit==mount.exitStatus()&& mount.exitCode()==0){
        qInfo()<<mount.arguments();
        qInfo()<<"Mount success!";
        return true;
    }else{
        qWarning()<<"An unknown error occurred! Mount process exit code:"<<mount.exitCode();
        mountstr=mount.errorString();
        mounterr=mount.readAllStandardError();
        m_getmountpoint=false;
        qInfo()<<__LINE__<<__func__<<"挂载输出"<<mountstr;
        qInfo()<<__LINE__<<__func__<<"挂载err输出"<<mounterr;
        getmountpoint();
        if(m_getmountpoint==true){
            qInfo()<<__LINE__<<__func__<<"U盘被挂载";
            return true;
        }
        else{
            emit makeFinish("fail");
            qInfo()<<"挂载失败";
            return false;
        }
    }

}

void SystemDbusRegister::isMakingSucess()
{
    QProcess lsblk;
    lsblk.start("lsblk", QStringList() << "-J" << "-o" << "NAME,TYPE,MOUNTPOINT,TRAN");
    lsblk.waitForFinished();
    if (lsblk.exitStatus() == QProcess::NormalExit && lsblk.exitCode() == 0) {
      qInfo() << "lsblk successfully ";
    } else {
      qWarning() << "Failed to lsblk" 
                << "Exit code:" << lsblk.exitCode()
                << "Error:" << lsblk.readAllStandardError();
    }
    qInfo()<<"isMakeingSucess检测中___________";
    QJsonDocument doc = QJsonDocument::fromJson(lsblk.readAllStandardOutput());
    QJsonArray devices = doc.object()["blockdevices"].toArray();
    qInfo()<<"devices检测中___________";
    for (const QJsonValue &deviceVal : devices) {
        QJsonObject device = deviceVal.toObject();
        
        // 检查是否为USB设备
        if (device["type"].toString() == "disk" &&
            device["tran"].toString() == "usb") {
            
            QString devName = "/dev/" + device["name"].toString();
            qInfo()<<device<<"______________";
            // 情况1：目标是磁盘设备
            qInfo()<<devName << "____________" <<uDiskPath;
            if (uDiskPath == devName) {
                    m_isdeviceexist = true;
                    qInfo()<<"目标是磁盘设备";
                    return;
    
            }
            
            // 情况2：目标是分区设备
            QJsonArray children = device["children"].toArray();
            for (const QJsonValue &childVal : children) {
                QString partName = "/dev/" + childVal.toObject()["name"].toString();
                qInfo() <<  partName <<"____________"<< uDiskPath;
                if (partName == uDiskPath) {
                    m_isdeviceexist = true;
                    qInfo()<<"目标是分区设备";
                    return;
                }
            }
        }
    }
}

void  SystemDbusRegister::getmountpoint()
{
    m_getmountpoint = false;
    qInfo()<<"in getmountpoint function";
    QProcess lsblk;
    qInfo()<<"lsblk__start";
    lsblk.start("lsblk", QStringList() << "-J" << "-o" << "NAME,TYPE,MOUNTPOINT,TRAN");
    qInfo()<<"lsblk"<< "-J" << "-o" << "NAME,TYPE,MOUNTPOINT,TRAN";
    lsblk.waitForFinished();

    QJsonDocument doc = QJsonDocument::fromJson(lsblk.readAllStandardOutput());
    if (doc.isNull()) {
      qWarning() << "Failed to parse lsblk output";
      return;
    }
    QJsonArray devices = doc.object()["blockdevices"].toArray();
    for (const QJsonValue &deviceVal : devices) 
    {

      QJsonObject device = deviceVal.toObject();

      qInfo() << "_______进入device_______";
      // 检查是否为USB磁盘
      if (device["type"].toString() == "disk" && device["tran"].toString() == "usb") {

          QString devName = "/dev/" + device["name"].toString();
          qInfo() << device["type"].toString() << device["tran"].toString() ;
          // 设备本身已挂载
          if ( !device["mountpoint"].isNull() && uDiskPath == devName) {
              mountpath = device["mountpoint"].toString();
              m_getmountpoint = true;
              qInfo() << "Device mounted at:" << mountpath;
              return;
          }

          // 检查分区
          QJsonArray children = device["children"].toArray();
          for (const QJsonValue &childVal : children) {
              QJsonObject child = childVal.toObject();
              QString partName = "/dev/" + child["name"].toString();
              qInfo() <<"partName___"<< partName;
              mountpath = child["mountpoint"].toString();
              m_getmountpoint = true;
              qInfo() << "Partition mounted at:" << mountpath;
              uDiskPath = partName;
              qInfo() << "uDiskPath is:" << uDiskPath;
              return;
            }
      }
    }
    qWarning() << "No matching mounted USB device found:" << uDiskPath;
}

QJsonArray SystemDbusRegister::QStringToJsonArray(const QString jsonString){
    QJsonParseError err;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toLocal8Bit().data(),&err);
    if(jsonDocument.isNull())
    {
        qDebug()<< "JsonDocument is NULL.Origin data is:"<< jsonString.toLocal8Bit().data();

    }
    if(err.error != QJsonParseError::NoError){
        qDebug()<<"Parase json"<<jsonString<<" error:"<<err.error;
        //TODO：这里的错误处理后期还可以优化,目前处理错误了就会调用exit()退出程序
        qInfo()<<"U盘被移除";
        //        exit(-1);
    }
    QJsonObject obj = jsonDocument.object();
    return obj["blockdevices"].toArray();
}

void SystemDbusRegister::checkmd5(){
    qInfo("开始检测MD5");
    m_isMd5Finish=false;
    ismakesuccess=false;
    md5=new QProcess();
    connect(md5,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(readStandardOutput(int,QProcess::ExitStatus)));
    QDir::setCurrent(mountpath);
    qDebug()<<QDir::currentPath();
    checkMd5FinishTimer->start(2000);
    md5->start("md5sum",QStringList{"-c","md5sum.txt"});
    connect(md5, &QProcess::readyReadStandardOutput, this, [=]() {
        MD5str=this->md5->readAllStandardOutput();
        qDebug()<<"calculating md5: "<<MD5str;
    });
}

void SystemDbusRegister::readStandardOutput(int exitCode,QProcess::ExitStatus exitStatus){
    qDebug()<<"md5退出"<<exitCode;
    if(exitStatus==QProcess::NormalExit && exitCode==0){
        if(MD5str.isEmpty()){
            qDebug()<<__LINE__<<__func__<<"输出为空"<<MD5str;
            QDir::setCurrent(QDir::rootPath());
            qDebug()<<QDir::currentPath();
            ismakesuccess=false;
        }else{
            if (MD5str.contains(tr("Fail")) || MD5str.contains(tr("FAILED")))
            {
                qDebug()<<__LINE__<<__func__<<"有错误字符"<<MD5str;
                qDebug()<<"md5 fail!";
                QDir::setCurrent(QDir::rootPath());
                qDebug()<<__LINE__<<__func__<<QDir::currentPath();
                ismakesuccess=false;
            }
            else{
                qDebug()<<__LINE__<<__func__<<"完全正确"<<MD5str;
                QDir::setCurrent(QDir::rootPath());
                qDebug()<<__LINE__<<__func__<<QDir::currentPath();
                ismakesuccess=true;
            }
        }

        qDebug()<<"md5 success!";
    }else{
        QDir::setCurrent(QDir::rootPath());
        qInfo()<<__func__<<__LINE__<<md5->errorString();
        qDebug()<<__LINE__<<__func__<<QDir::currentPath();
        qDebug()<<"md5 fail!";
    }
    qDebug()<<"md5退出"<<exitCode;
    m_isMd5Finish=true;

}

void SystemDbusRegister::checkmd5finish(){
    qDebug()<<"in checkmd5finish function";
    if(m_isMd5Finish==true){
        checkMd5FinishTimer->stop();
        if(ismakesuccess==true){
            qDebug()<<"md5 checked success,begin to umount";
            if(umountPath(mountpath)){
                emit makeFinish("success");
            }
            qDebug()<<"status"<<m_isFinishstatus;
        }else if(ismakesuccess==false){
            qDebug()<<"md5 checked failed,begin to umount";
            if(umountPath(mountpath)){
                emit makeFinish("fail");
            }
            qDebug()<<"ismakesuccess"<<ismakesuccess;
            qDebug()<<"status"<<m_isFinishstatus;
        }
    }
}

QStringList SystemDbusRegister::findPartitions(QString devname) {
    qInfo() << "========== 开始查找分区 ==========";
    qInfo() << "目标设备名: " << devname;
    
    QStringList partitions;
    QProcess process;

    process.start("lsblk", QStringList() << "-J" << "-o" << "NAME,TYPE,MOUNTPOINT,FSTYPE,SIZE");
    process.waitForFinished();

    QJsonDocument doc = QJsonDocument::fromJson(process.readAllStandardOutput());
    if (doc.isNull()) {
        qWarning() << "解析lsblk输出失败";
        qInfo() << "========== 查找分区结束（失败）==========";
        return partitions;
    }

    QJsonObject root = doc.object();
    QJsonArray devices = root["blockdevices"].toArray();
    
    qInfo() << QString("系统中共有 %1 个块设备").arg(devices.size());

    bool deviceFound = false;
    
    // 查找目标设备及其分区
    for (const QJsonValue &deviceVal : devices) {
        QJsonObject device = deviceVal.toObject();
        QString deviceName = device["name"].toString();
        QString deviceType = device["type"].toString();
        
        qInfo() << QString("检查设备: %1 (类型: %2)").arg(deviceName).arg(deviceType);
        
        if (deviceName == devname && deviceType == "disk") {
            deviceFound = true;
            QString deviceSize = device["size"].isNull() ? "未知" : device["size"].toString();
            qInfo() << QString("找到目标磁盘: %1, 大小: %2").arg(deviceName).arg(deviceSize);
            
            QJsonArray children = device["children"].toArray();
            qInfo() << QString("该磁盘共有 %1 个子设备").arg(children.size());
            
            int partCount = 0;
            int mountedPartCount = 0;
            
            for (const QJsonValue &childVal : children) {
                QJsonObject child = childVal.toObject();
                QString childName = child["name"].toString();
                QString childType = child["type"].toString();
                QString childFsType = child["fstype"].isNull() ? "未知" : child["fstype"].toString();
                QString childSize = child["size"].isNull() ? "未知" : child["size"].toString();
                QString childMountPoint = child["mountpoint"].isNull() ? "未挂载" : child["mountpoint"].toString();
                //bool isMounted = !child["mountpoint"].isNull();
                
                if (childType == "part") {
                    partCount++;
                    qInfo() << QString("  分区 %1: /dev/%2, 文件系统: %3, 大小: %4, 挂载点: %5")
                               .arg(partCount)
                               .arg(childName)
                               .arg(childFsType)
                               .arg(childSize)
                               .arg(childMountPoint);
                    
                     mountedPartCount++;
                     QString partPath = "/dev/" + childName;
                     partitions << partPath;
                     uDiskPath = partPath;
                     qInfo() << QString("    → 已挂载，加入卸载列表");
                   // }
                } else {
                    qInfo() << QString("  子设备: /dev/%1 (类型: %2, 非分区，跳过)").arg(childName).arg(childType);
                }
            }
            
            qInfo() << QString("该磁盘共有 %1 个分区，其中 %2 个已挂载").arg(partCount).arg(mountedPartCount);
            break;
        }
    }
    
    if (!deviceFound) {
        qWarning() << QString("未找到目标磁盘设备: %1").arg(devname);
        qInfo() << "可用磁盘设备列表:";
        for (const QJsonValue &deviceVal : devices) {
            QJsonObject device = deviceVal.toObject();
            if (device["type"].toString() == "disk") {
                qInfo() << QString("  - %1").arg(device["name"].toString());
            }
        }
    }

    qInfo() << QString("========== 查找分区结束 ==========");
    qInfo() << QString("找到 %1 个需要卸载的分区: %2").arg(partitions.size()).arg(partitions.join(", "));
    
    return partitions;
}

bool SystemDbusRegister::unmountPartition(const QString& partition) {
    QProcess process;
    qInfo() << "开始执行指令：umount" << "-v" << partition;
    process.start("umount", QStringList()<< "-v" << partition  );
    process.waitForFinished(-1);

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
      qInfo() << "Successfully unmounted" << partition;
      return true;
    } else {
      qWarning() << "Failed to unmount" << partition
                << "Exit code:" << process.exitCode()
                << "Error:" << process.readAllStandardError();
      return false;
    }
}

bool SystemDbusRegister::umountPath(QString Path)
{
    qInfo() << "Unmounting device:" << Path;
    

    // 获取设备名（如从/dev/sda1得到sda）
    QString devname = Path.mid(5); // 去掉"/dev/"
    if (devname.contains(QRegExp("[0-9]$"))) {
      devname = devname.left(devname.size()-1);
    }

    // 查找并卸载所有分区
    QStringList partitions = findPartitions(devname);
    if(!partitions.isEmpty())
    {
        uDiskPath = partitions.first();
        qInfo() << "uDiskPath " << "is " << partitions.first() ;
    }
    // 如果传入的是分区（如/dev/sda1），也把它加入卸载列表
    if (Path.startsWith("/dev/") && QRegExp("[0-9]$").indexIn(Path) != -1) {
        if (!partitions.contains(Path)) {
            partitions.append(Path);
            qInfo() << "Added target partition to unmount list:" << Path;
        }
    }
    bool allSuccess = true;

    for (const QString &partition : partitions) {
      if (!unmountPartition(partition)) {
          allSuccess = false;
          qWarning() << "Failed to unmount partition" << partition;
      }
    }

    if (partitions.isEmpty()) {
      qInfo() << "No mounted partitions found for device" << Path;
      // 卸载设备本身
      if (unmountPartition(Path)) {
          qInfo() << "Device " << Path << "was mounted and has been unmounted";
          m_getmountpoint = false;
    }
      return true;
    }

    if (!allSuccess) {
      emit makeFinish("fail");
      qInfo() << "Failed to unmount some partitions";
    }else{
        m_getmountpoint = false;
    }
    return allSuccess;
}
