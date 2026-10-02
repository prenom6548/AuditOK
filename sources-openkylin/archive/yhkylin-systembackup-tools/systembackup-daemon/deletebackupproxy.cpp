/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "deletebackupproxy.h"
#include <QProcess>
#include <QDateTime>
#include <QRegularExpression>
#include <QDebug>
#include "../common/utils.h"

IMPLEMENT_DYNCREATE(DeleteBackupProxy)

DeleteBackupProxy::DeleteBackupProxy()
{}

DeleteBackupProxy::~DeleteBackupProxy()
{}

void DeleteBackupProxy::doWorkEx()
{
    qDebug() << "DeleteBackupProxy::doWorkEx invoke begin";

    // 删除//snapshots/backup.xml对应项
    if (!deleteXmlBackupPoint()){
        emit this->workResult(false);
        return;
    }
    // 同时删除唯一识别文件中对应的条目
    Utils::deleteBackupUniqueRecord(m_backupWrapper.m_backupName);

    // 删除成功信号
    emit this->workResult(true);

    qDebug() << "DeleteBackupProxy::doWorkEx invoke end";
}

/**
 * @brief 删除/.backup/snapshots/*.xml 对应项
 */
bool DeleteBackupProxy::deleteXmlBackupPoint()
{
    qDebug() << "delete Xml BackupPoint invoke begin,## dest path ##" << m_backupWrapper.m_prefixDestPath;

    QString xmlPath;
    if (m_backupWrapper.m_backupType == 1 && m_backupWrapper.m_iPosition==0){
        // xmlPath = m_backupWrapper.m_prefixDestPath + SYSTEM_XML;
        xmlPath = Utils::getBackupRootPath() + SYS_BACKUP_XML_PATH;
    }else if (m_backupWrapper.m_backupType == 1 && m_backupWrapper.m_iPosition==1) {
        xmlPath = m_backupWrapper.m_prefixDestPath + SYS_BACKUP_XML_PATH;
    }else if(m_backupWrapper.m_iPosition==0 && m_backupWrapper.m_backupType == 0)
    {
        xmlPath = m_backupWrapper.m_prefixDestPath + USER_XML;
    }else{
        xmlPath = m_backupWrapper.m_prefixDestPath + USER_XML;
    }

    xmlPath.replace("//", "/");
    qDebug()<<"in deleteXML backuppoint ,xml path ### "<< xmlPath;
    ParseBackupList parse(xmlPath);
    // 查询节点
    qDebug()<<"uuid ##### "<< m_backupWrapper.m_uuid;
    m_backupPoint = parse.findBackupPointByUuid(m_backupWrapper.m_uuid);

    // 移动设备系统备份点
    if (m_backupWrapper.m_backupType == 1 && m_backupWrapper.m_iPosition==1) {
        QList<ParseBackupList::BackupPoint> backupList = parse.getBackupPointList();
        int count = 0;
        for (const auto& point : backupList) {
            qDebug() << "标志位" << point.m_iPosition;
            if (point.m_iPosition == 1) {
                count = count + 1;
            }
        }
        if (count == 1) {
            // 是最后一个移动设备系统备份点
            m_isLastMobileBackupPoint = true;
            qDebug() << "是否是最后一个移动设备系统备份点：" << m_isLastMobileBackupPoint;
        } else {
            m_isLastMobileBackupPoint = false;
            qDebug() << "是否是最后一个移动设备系统备份点：" << m_isLastMobileBackupPoint;
        }
    }

    //查询用户数据备份对应的目录commit哈希
    if(m_backupWrapper.m_backupType ==0)
    {
        //获取用户备份信息
        parse.getDataCommitHashMap(m_backupPoint.m_systemHash,m_dataCommitHashMap,m_backupWrapper.m_uuid);
        qDebug()<<"get user data commit hash info , hash size "<<m_dataCommitHashMap.size();
        if (m_dataCommitHashMap.size() == 0)
        {
            qCritical("no backup commit hash found");
            return false;
        }
    }

    //查询系统备份对应的目录commit哈希
    if(m_backupWrapper.m_backupType ==1)
    {
        //获取系统备份信息
        parse.getXmlCommitHashMap(m_backupPoint.m_systemHash, m_systemCommitHashMap, m_backupWrapper.m_uuid);
        qDebug()<<"get system commit hash info , hash size "<<m_systemCommitHashMap.size();
        if (m_systemCommitHashMap.size() == 0)
        {
            qCritical("no backup commit hash found");
            return false;
        }
    }

    //先删commit,再删xml文件，否则不好排查
    if(!deleteBackupPointCommits()){
        return false;
    }
    // 删除节点
    if (m_backupPoint.isNull())
    {
        // 系统更新的备份点独立了出来
        if (m_backupWrapper.m_uuid == AUTO_BACKUP_UUID)
        {
            xmlPath = Utils::getBackupRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
            xmlPath.replace("//", "/");
            ParseBackupList xmlparse(xmlPath);
            // 查询节点
            m_backupPoint = xmlparse.findBackupPointByUuid(m_backupWrapper.m_uuid);
            if (xmlparse.deleteItem(m_backupWrapper.m_uuid) != ParseBackupList::SUCCESS) {
                qCritical() << QString("failed to delete %1 from backuplist.xml or sys_update_backuplist.xml!").arg(m_backupWrapper.m_uuid);
                return false;
            }
        }
    }else{
        if (!m_backupWrapper.m_uuid.contains(QRegularExpression("^.*-.*-.*-.*-.*$")) )
        {
            qDebug()<<"uuid format error";
        }

         if( parse.deleteItem(m_backupWrapper.m_uuid) != ParseBackupList::SUCCESS) {
            qCritical() << QString("failed to delete %1 from user xml!").arg(m_backupWrapper.m_uuid);
            return false;
        }
         qDebug()<<"delete backup successfully "<< m_backupWrapper.m_uuid;
    }

    qDebug() << "delete Xml BackupPoint invoke end";
    return true;
}

/**
 * @brief 删除ostree 中对应的branch和commit
 */
bool DeleteBackupProxy::deleteBackupPointCommits()
{
    qDebug() << "DeleteBackupProxy::deleteBackupPointDir function deleteBackupPointDir";

    // 检测ostree仓库是否存在
    m_pOstree = new OstreeInterface(this);
    QString backupPath = m_backupWrapper.m_prefixDestPath;
    QString branch= m_backupPoint.m_branchName;

    qDebug()<<"backup type #### "<< m_backupWrapper.m_backupType << "  backup path "<< backupPath;
    //用户数据备份
    if(m_backupWrapper.m_backupType == 0 )
    {
        repoPath = m_pOstree->getRepoPath(backupPath, OstreeInterface::BackupType::USER);
        if(!m_pOstree->checkOstreeRepo(backupPath,OstreeInterface::BackupType::USER))
        {
            qDebug()<<"no repo found in "<< repoPath;
            return false;
        }

        QString tempRepo;
        // 数据备份仓库路径去除单引号，避免commit时报错
        if( repoPath.startsWith("\'") && repoPath.endsWith("\'") )
        {
            tempRepo = repoPath.mid(1,repoPath.length()-2);
        }

        //删除branch 命令  ostree refs --delete branch-20250722091302 --repo /data/.backup/repo_user
        if (!runOstreeCommand({"refs", "--delete", branch, "--repo" ,tempRepo}))
        {
           qWarning() << "Failed to delete branch:" << branch;
           return false;
        }

        qDebug() << "HashMap size## "<< m_dataCommitHashMap.size() ;

        //删除hash 命令 ostree prune --delete-commit a9a0694b4b3d72d74d1808fdee9819f27acdad52308a85f756a00d9c24574945 --repo /data/.backup/repo_user
        for (auto it = m_dataCommitHashMap.constBegin(); it != m_dataCommitHashMap.constEnd(); ++it)
        {
            qDebug() << "Path:" << it.key()
                     << "Hash:" << it.value().hash
                     << "IsDirectory:" << it.value().isDirectory;

            if (!runOstreeCommand({"prune", "--delete-commit", it.value().hash , "--repo", tempRepo}))
            {
               qWarning() << "Failed to delete commit :" << it.value().hash;
               return false;
            }
        }
    }else if (m_backupWrapper.m_backupType == 1){
        qDebug()<<"删除系统备份";

        //设置工作类型为备份点管理
        m_pOstree->setTaskType(OstreeInterface::TaskType::BACKUPPOINT_MANAGEMENT);
        qDebug() << "设置工作类型为:备份点管理" ;

        //检查repo点是否存在
        backupPath += "/.backup/";
        QString tmpBackupPath;
        if(m_backupWrapper.m_iPosition==1) {
            tmpBackupPath = backupPath;
        }
        else {
            tmpBackupPath = "";
        }
        if(!m_pOstree->checkOstreeRepo(tmpBackupPath, OstreeInterface::BackupType::SYSTEM))
        {
            qDebug() << "备份点ostree仓库目录不存在";
            m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);
            return false;
        }

        //获取ostree仓库目录
        QString repoPath;
        if(m_backupWrapper.m_iPosition==1) {
            repoPath = m_pOstree->getRepoPath(backupPath, OstreeInterface::BackupType::SYSTEM);
        }
        else {
            repoPath = m_pOstree->getRepoPath("", OstreeInterface::BackupType::SYSTEM);
        }

        QString tempRepo;
        // 数据备份仓库路径去除单引号，避免commit时报错
        if( repoPath.startsWith("\'") && repoPath.endsWith("\'") )
        {
            tempRepo = repoPath.mid(1,repoPath.length()-2);
        }

        QStringList pathNameList = {"ovl", "home"};

        // 第一步：删除分支，解除引用
        QStringList refsArgs = m_pOstree->getRefsArgs(tempRepo, m_backupPoint.m_branchName);
        m_pOstree->setOperation(OstreeInterface::Operation::REFS);
        qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::REFS);
        m_pOstree->start(refsArgs, true);
        // 第二步：删除commit对应的目录
        for (auto dirPath : pathNameList) {
            QString commitID = m_backupPoint.m_systemCommitHashMap.value(dirPath);
            QStringList pruneArgs = m_pOstree->getPruneArgs(tempRepo, commitID);
            m_pOstree->setOperation(OstreeInterface::Operation::PRUNE);
            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::PRUNE);
            m_pOstree->start(pruneArgs, true);
        }

//        // 最后一个移动设备系统备份点，删除不可变os部分
//        if (m_backupWrapper.m_iPosition==1 && m_isLastMobileBackupPoint) {
//            // 第一步：删除os-branch分支，解除引用
//            QStringList refsArgs = m_pOstree->getRefsArgs(repoPath, m_backupPoint.m_systemCommitHashMap.value("os-branch"));
//            m_pOstree->setOperation(OstreeInterface::Operation::REFS);
//            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::REFS);
//            m_pOstree->start(refsArgs, true);
//            // 第二步：删除commit对应的目录
//            QString commitID = m_backupPoint.m_systemCommitHashMap.value("os");
//            QStringList pruneArgs = m_pOstree->getPruneArgs(repoPath, commitID);
//            m_pOstree->setOperation(OstreeInterface::Operation::PRUNE);
//            qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::PRUNE);
//            m_pOstree->start(pruneArgs, true);
//        }

        //执行完毕，设置工作类型为空闲
        m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);
    }

    QString curTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    if (   int(BackupType::BACKUP_DATA) == m_backupPoint.m_type
        || int(BackupType::INC_BACKUP_DATA) == m_backupPoint.m_type) {
        //Utils::writeBackupLog(m_backupPoint.m_time + "," + m_backupWrapper.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ",,," + QString::number(m_backupWrapper.m_frontUid)+ ",," + m_backupPoint.m_backupName);
        Utils::writeBackupLog(curTime + "," + m_backupPoint.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ","+ m_backupWrapper.m_note + "," + m_backupPoint.m_size+ ",," + m_backupWrapper.m_backupName);
    } else {
        //Utils::writeBackupLog(m_backupPoint.m_time + "," + m_backupWrapper.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ",,,,,," + m_backupPoint.m_backupName);
        Utils::writeBackupLog(curTime + "," + m_backupPoint.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ","+ m_backupWrapper.m_note + "," + m_backupPoint.m_size+ ",," + m_backupWrapper.m_backupName);

    }
    return true;

    qDebug() << "DeleteBackupProxy::deleteBackupPointDir invoke end";
}

bool DeleteBackupProxy::runOstreeCommand(const QStringList &args)
{
    qDebug()<<"ostree cmd "<< args;
    QProcess process;
    process.setProgram("ostree");
    process.setArguments(QStringList() << args );
    process.start();

    connect(&process, &QProcess::readyReadStandardError, [&](){
        qDebug() << "[OSTree execute Error] " << process.readAllStandardError();
    });

    //  ostree.start();
    if (!process.waitForFinished(-1) || process.exitCode() != 0) {
        qWarning() << "OSTree exec failed:" << process.errorString();
        return false;
    }
    return true;
}
