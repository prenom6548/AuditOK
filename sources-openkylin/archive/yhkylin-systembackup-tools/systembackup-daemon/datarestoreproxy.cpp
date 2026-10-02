/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "datarestoreproxy.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <unistd.h>
#include <sys/reboot.h>
#include "../common/utils.h"
#include "mymountproxy.h"
#include "translations.h"
#include <QRandomGenerator>
#include <unistd.h>

IMPLEMENT_DYNCREATE(DataRestoreProxy)

/**
 * @brief 构造函数
 */
DataRestoreProxy::DataRestoreProxy()
{
    m_bSuccess = false;
    m_isOem = false;
    m_extraProcess = nullptr;
}

/**
 * @brief 析构函数
 */
DataRestoreProxy::~DataRestoreProxy()
{
}

/**
 * @brief 环境检测
 * @return false,检测失败;true,检测成功
 */
bool DataRestoreProxy::checkEnvEx()
{
    qDebug() << "DataRestoreProxy::checkEnvEx invoke begin";

    // 1、检测xml中的还原点是否还存在
    QString xmlPath;
    qDebug()<< "positon is "<< m_backupWrapper.m_iPosition;
    if(m_backupWrapper.m_iPosition == 0 )//本地存储
    {
        xmlPath = m_backupWrapper.m_prefixDestPath + USER_XML;
    }else{
        //U盘路径
        xmlPath = m_backupWrapper.m_prefixDestPath + USER_XML;
    }
   // QString xmlPath = Utils::getBackupRootPath() + USER_BACKUP_XML_PATH;
    qDebug()<<"restore check path "<< xmlPath;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    qDebug()<<__LINE__<<__func__<<"数据备份点："<<m_backupWrapper.m_uuid;
    m_backupPoint = parse.findBackupPointByUuid(m_backupWrapper.m_uuid);
    if (m_backupPoint.isNull())
    {
        qCritical("xml中还原点不存在");
        emit checkResult(int(BackupResult::INC_NOT_FOUND_DIR));
        return false;
    }

    //查询对应的目录commit哈希
    parse.getDataCommitHashMap(m_backupPoint.m_systemHash,m_dataCommitHashMap,m_backupWrapper.m_uuid);
    if (m_dataCommitHashMap.size() == 0)
    {
        qCritical("存储点中没有对应哈希");
        emit checkResult(int(BackupResult::CHECK_COMMITHASH_FAIL));
        return false;
    }
    // 检测ostree仓库是否存在
    m_pOstree = new OstreeInterface(this);
    m_pOstree->setOperation(OstreeInterface::Operation::INIT);
    qDebug() << "设置操作类型：" << int(OstreeInterface::Operation::INIT);
    QString repoDir = m_backupWrapper.m_prefixDestPath;
    if(!m_pOstree->checkOstreeRepo(repoDir,OstreeInterface::BackupType::USER))
    {
        return false;
    }

    m_curUuid = m_backupWrapper.m_uuid;

    emit checkResult(int(BackupResult::CHECK_ENV_SUCCESS));

    qDebug() << "DataRestoreProxy::checkEnvEx invoke end";
    return true;
}

/**
 * @brief 执行还原逻辑
 */
void DataRestoreProxy::doWorkEx()
{
    qDebug() << "DataRestoreProxy::doWorkEx invoke begin";

    // 1 校验
    if (!checkEnvEx())
        return ;

    // 2、还原系统
    restoreData();

    qDebug() << "DataRestoreProxy::doWorkEx invoke end";
}

/**
 * @brief 备份准备
 * @return true,准备成功；false，准备失败
 */
bool DataRestoreProxy::doPrepare()
{
    qDebug() << "DataRestoreProxy::doPrepare invoke begin";


    // 2、停止安全初始化服务，以防过高的CPU占有率，因为还原时安全初始化服务会逐个文件打标记，造成cpu占有率超高系统卡顿

    // 停止网络服务，以防网络更新
    //Utils::stopNetwork();

    qDebug() << "DataRestoreProxy::doPrepare invoke end";
    return true;
}

/**
 * @brief 将dst.img文件挂载到/backup/imgbackup目录
 * @return
 */
bool DataRestoreProxy::mountImg()
{
    // 自定义路径系统备份，需要先将dst.img文件挂载到/backup/imgbackup目录
    // 1、检测目录/backup/imgbackup是否存在，不存在则创建此目录
    QString dstImgMountPath = Utils::getBackupRootPath() + BACKUP_IMGBACKUP_PATH;
    dstImgMountPath.replace("//", "/");
    Utils::mkpath(dstImgMountPath);

    // 2、先卸载/backup/imgbackup上的mount
    MountBackupProcess *processMount = new MountBackupProcess(this);
    processMount->umount(dstImgMountPath);

    // 3、将img文件挂载到/backup/imgbackup上
    if (!processMount->mount(m_imgFileName, dstImgMountPath)) {
        emit checkResult(int(BackupResult::RESTOREDIR_PREPARE_FAILED));
        delete processMount;
        return false;
    }

    m_srcPath = dstImgMountPath;

    delete processMount;

    return true;
}

/**
 * @brief 根据场景获取rsync命令参数
 * @param scene，场景
 * @return 组装好的rsync的参数信息
 */
QStringList DataRestoreProxy::getRsyncArgs(CustomizeSystemRestoreScene scene)
{
    QStringList args;
    args << "-avAHXr";
    args << "--info=progress2";
    args << "--no-inc-recursive";
    args << "--ignore-missing-args";
    args << "--delete";

    QStringList excludes;

    switch (scene) {
    case CustomizeSystemRestoreScene::RESTORE_SYSTEM_WITH_DATA :
        args << "--exclude=/home";
        args << "--exclude=/root";
        args << "--exclude=/data";

        if (m_curUuid == FACTORY_BACKUP_UUID) {
            qDebug() << "FACTORY_BACKUP_UUID";
//            if (Utils::isDirExist(m_srcPath + "/home/oem"))
            {
                qDebug() << "Exists /home/oem";
                args << "--exclude=/etc/lightdm/lightdm.conf";
                args << "--exclude=/etc/localtime";
                args << "--exclude=/usr/share/zoneinfo";
                args << "--exclude=/etc/default/locale";
                args << "--exclude=/usr/share/i18n";
            }
        }
        // 保留指纹数据，用户密码、角色、权限、生物识别等信息不需要改变
        args << "--exclude=/var/lib/biometric-auth";
        args << "--exclude=/data/sec_storage_data";
        args << "--exclude=/etc/passwd";
        args << "--exclude=/etc/shadow";
        args << "--exclude=/etc/group";
        args << "--exclude=/etc/gshadow";
        args << "--exclude=/etc/sudoers";
        args << "--exclude=/data/home";
        args << "--exclude=/data/root";

        // 云图片作为桌面背景的路径属于用户数据
        args << "--exclude=/var/lib/AccountsService";

        // 域用户相关信息，还原后保持不退域
        args << "--exclude=/etc/sssd";
        args << "--exclude=/var/lib/sss";
        args << "--exclude=/usr/share/sssd";
        args << "--exclude=/etc/ipa";
        args << "--exclude=/etc/krb5.keytab";
        args << "--exclude=/etc/krb5.conf";
        args << "--exclude=/var/lib/ipa-client";
        args << "--exclude=/etc/nsswitch.conf";
        args << "--exclude=/etc/pam.d";
        args << "--exclude=/etc/hosts";
        args << "--exclude=/etc/hostname";
        args << "--exclude=/etc/hedron";
        args << "--exclude=/etc/kcm";
        args << "--exclude=/usr/hedron/hedronagent";
        args << "--exclude=/etc/.kyinfo";
        args << "--exclude=/etc/LICENSE";
        args << "--exclude=/etc/ssl/certs";
        args << "--exclude=/usr/share/ca-certificates";
        args << "--exclude=/etc/NetworkManager";
        args << "--exclude=/var/lib/pam";
        args << "--exclude=/etc/kim";
        args << "--exclude=/var/lib/kim";
        args << "--exclude=/etc/systemd/system/multi-user.target.wants/kimbackend.service";
        args << "--exclude=/var/lib/polkit-1/localauthority/40-domain.d";
        args << "--exclude=/lib/*/libsss_sudo.so";

        // 安装kylin
        args << "--exclude=/usr/share/applications/kylin-os-installer.desktop";
        args << "--exclude=*/.local/share/applications/kylin-os-installer.desktop";
        args << "--exclude=/etc/xdg/autostart/kylin-os-installer.desktop";

        // 系统更新的下面两个目录不要还原
        if (m_curUuid == AUTO_BACKUP_UUID) {
          //  args << "--exclude=/var/lib/kylin-system-updater";
           // args << "--exclude=/var/cache/kylin-system-updater";
            args << "--exclude=/var/lib/kylin-system-updater/remain";

        }

        // 此处不要break，因为还需要排除SYSTEM_RESTORE中的项

    case CustomizeSystemRestoreScene::SYSTEM_RESTORE :
        // 还原工具不还原自身
        args << "--exclude=/usr/bin/kylin-backup-daemon";
        args << "--exclude=/usr/bin/backup_mount_fstab";
        args << "--exclude=/usr/bin/restore-system";
        args << "--exclude=/usr/share/initramfs-tools/hooks/kylin-backup-hooks";
        args << "--exclude=/usr/share/initramfs-tools/scripts/init-bottom/kylin-backup";
        args << "--exclude=/usr/share/initramfs-tools/conf-hooks.d/kylin-backup";
        args << "--exclude=/etc/extra_backup_or_restore.conf";

        if (Utils::getBackupRootPath() != CASPER_ROOT_PATH) {
            // 试用模式时可以还原/target下面的备份还原工具
            args << "--exclude=/usr/bin/backup-daemon";
            args << "--exclude=/usr/bin/kybackup";
        }
        args << "--exclude=/usr/bin/mount_fstab_efi";
        args << "--exclude=/usr/bin/backup-auto-efi";
        args << "--exclude=/usr/bin/backup-auto";
        args << "--exclude=/usr/bin/rsync";
        args << "--exclude=/usr/share/rsync";
        args << "--exclude=/usr/share/initramfs-tools/hooks/kybackup-hooks";
        args << "--exclude=/usr/share/initramfs-tools/scripts/*/kybackup";

        // 还原后仍然保持激活状态
        args << "--exclude=/etc/LICENSE";
        args << "--exclude=/etc/.kyinfo";
        args << "--exclude=/etc/.kyactivation";
        args << "--exclude=/etc/.kyhwid";

        args << "--exclude=/usr/share/plymouth";
        // 文件安全箱
        args << "--exclude=/data/security-dir";

        // 自定义备份的路径也需要跳过，不进行还原
        Utils::excludeCustomizePath(excludes);
        args << "--exclude=*/backup/snapshots";
        for (const QString& item : excludes) {
            args << QString("--exclude=") + item;
        }

        args << "--exclude-from" << m_excludeUserFile;
        args << "--files-from" << m_userFile;

        break ;
    case CustomizeSystemRestoreScene::RESTORE_EXTRA_DATA :
        args << "--files-from" << m_extraUserFile;

        break ;
    default:
        break ;
    }

    return args;
}

/**
 * @brief 系统还原
 */
void DataRestoreProxy::restoreData()
{
    qDebug() << "DataRestoreProxy::restoreData invoke begin";

    //接入ostree
    m_pOstree = new OstreeInterface(this);
    //设置工作类型为还原
    m_pOstree->setTaskType(OstreeInterface::TaskType::RESTORE);
    qDebug() << "设置工作类型为:还原" ;
    QString repoDir = m_backupWrapper.m_prefixDestPath;
    repoPath = m_pOstree->getRepoPath(repoDir, OstreeInterface::BackupType::USER);

    //检查repo点是否存在
    if(!m_pOstree->checkOstreeRepo(repoPath,OstreeInterface::BackupType::USER))
    {
        //不存在检测repo失败
        emit checkResult(int(BackupResult::CHECK_REPO_FAIL));
    }

    qDebug() << "用户数据还原操作:开始";

    int m_progress = 0;
    m_totalItems = m_dataCommitHashMap.size();

    qDebug() << "HashMap size## "<< m_dataCommitHashMap.size() ;
    for (auto it = m_dataCommitHashMap.constBegin(); it != m_dataCommitHashMap.constEnd(); ++it)
    {

        qDebug() << "Path:" << it.key()
                 << "Hash:" << it.value().hash
                 << "IsDirectory:" << it.value().isDirectory;

        bool cur_isdir = it.value().isDirectory;

        //处理文件夹
        if(cur_isdir)
        {
            QString original_path = it.key();
            QString cur_hash = it.value().hash;

            QFileInfo fileInfo(original_path);
            QString filename = fileInfo.fileName();

            //以时间戳新建文件夹作为临时checkout目录

            QString tmp = Utils::createUuid();
            QString tmp_dir = generate16DigitRandomNumber();
            QString dest = QString("%1%2/").arg(TEMP_RESTORE_PATH).arg(tmp_dir);

            qDebug()<<"current dest "<< dest;
            // OSTree checkout到临时目录
            if (!ostreeCheckout(cur_hash, dest)) {
                qCritical() << "OSTree checkout failed for hash: " << cur_hash << " restore path "<< original_path;
                emit this->workResult(false);
                return;
            }

            // 如果是文件，则拼凑文件路径
            if(!cur_isdir){
                dest = QDir(dest).filePath(filename);
            }
            qDebug()<<"current dest "<< dest;
            // 使用rsync复制并保留属性
            if (!rsyncCopy(dest, original_path, cur_isdir)) {
                qCritical() << "Rsync failed from " << dest << " to " << original_path;
                emit this->workResult(false);
                return;
            }
            updateProgress(++m_progress);
        }
        else   //处理文件
        {
            QString original_path = it.key();
            QString cur_hash = it.value().hash;
            //以时间戳新建文件夹作为临时checkout目录
            QString tmp_dir = generate16DigitRandomNumber();
            QString dest = QString("%1%2/").arg(TEMP_RESTORE_PATH).arg(tmp_dir);
            qDebug()<<"current dest "<< dest;
            // OSTree checkout到临时目录
            if (!ostreeCheckout(cur_hash, dest)) {
                qCritical() << "OSTree checkout failed for hash: " << cur_hash << " restore path "<< original_path;
                emit this->workResult(false);
                return;
            }

            //根据文件原始路径，做硬链接
            QStringList filesPathList = original_path.split(";");
            qDebug()<<"filesPathList size :  "<< filesPathList.size();
            for(int i = 0 ; i < filesPathList.size() - 1; i++)
            {
                //原始文件路径
                QString filePath = filesPathList[i];

                //获取文件名
                QFileInfo fileInfo(filePath);
                QString filename = fileInfo.fileName();

                //先删除旧文件，然后还原新文件
                Utils::deleteFile(filePath);
                linkFile(dest + "/" + filename,filePath);

                //增加计数
                updateProgress(++m_progress);
            }

            //clearDirectory(dest);
        }
    }

    //删除TEMP_RESTORE_PATH下新建的临时文件夹,删除失败也不处理，不影响主操作

    clearDirectory(TEMP_RESTORE_PATH);

    qDebug() << "还原操作:结束";

   // 写操作日志
    QString curTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    Utils::writeBackupLog(curTime + "," + m_backupWrapper.m_uuid + "," + QString::number(m_backupWrapper.m_type) + ",,," + QString::number(m_backupWrapper.m_frontUid)+ "," + m_backupPoint.m_backupName);

    emit this->workResult(true);
    m_pOstree->setTaskType(OstreeInterface::TaskType::IDLE);
    qDebug() << "DataRestoreProxy::restoreSystem invoke end";
}

/**
 * @brief 还原额外数据
 */
void DataRestoreProxy::restoreExtraData()
{
    qDebug() << "DataRestoreProxy::restoreExtraData invoke begin";

    m_extraDir = Utils::getBackupRootPath() + m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/extra_data";
    m_extraDir.replace("//", "/");
    if (Utils::isDirEmpty(m_extraDir)){
        qDebug() << m_extraUserFile << " is empty";
        return ;
    }

    m_extraUserFile = Utils::getBackupRootPath() + m_backupPoint.m_path + BACKUP_SNAPSHOTS_PATH + "/" + m_curUuid + "/" + EXTRA_PATHS_USER_FILE;
    m_extraUserFile.replace("//", "/");
    if (!Utils::fileExists(m_extraUserFile)) {
        qDebug() << m_extraUserFile << " not exists";
        return ;
    }

    QStringList args = getRsyncArgs(DataRestoreProxy::RESTORE_EXTRA_DATA);

    m_srcPath = m_backupPath;
    QString destPath = Utils::getBackupRootPath() + "/";
    destPath.replace("//", "/");
    args << m_extraDir + "/";
    args << destPath;

    m_extraProcess = new RsyncPathToDirProcess(this);
    m_extraProcess->start(args, false);

    qDebug() << "DataRestoreProxy::restoreExtraData invoke end";
}

bool DataRestoreProxy::ostreeCheckout(QString commitHash,  QString targetDir)
{
    QProcess ostree;
    ostree.setProgram("ostree");
    /*
    ostree.setArguments({
       "checkout",
       "--repo=" + repoPath,
       commitHash,
       targetDir
    });
    */

    QString tempRepo;
    // 数据备份仓库路径去除单引号，避免commit时报错
    if( repoPath.startsWith("\'") && repoPath.endsWith("\'") )
    {
        tempRepo = repoPath.mid(1,repoPath.length()-2);
    }

    QStringList args={
        "checkout",
        "--repo", tempRepo,
        commitHash,
        targetDir,
    };


    // 打印完整命令
    QString fullCommand = "ostree " + args.join(" ");
    qDebug() << "Executing:" << fullCommand;

    connect(&ostree, &QProcess::readyReadStandardError, [&](){
        qDebug() << "[OSTree checkout Error] " << ostree.readAllStandardError();
    });

    // 设置参数并执行
    ostree.setArguments(args);

    ostree.start();
    if (!ostree.waitForFinished(-1)) {
       qCritical() << "OSTree process failed to finish when resotre## "<< targetDir <<" error## " << ostree.errorString();
       return false;
    }

    if (ostree.exitCode() != 0) {
       qCritical() << "OSTree checkout failed:"
                  << ostree.readAllStandardError();
       return false;
    }

    return true;
}

bool DataRestoreProxy::rsyncCopy(const QString& source, const QString& dest, bool isDir)
{
    //rsync -aHAXog --delete /path/to/A/ /path/to/B/
    //文件 rsync -aHAXog --force /data/config.xml /etc/app/config.xml
    QProcess rsync;
    rsync.setProgram("rsync");
    if(isDir){
       qDebug()<<"## it's directory ##";
       rsync.setArguments({
           "-aHAXog",       // 归档模式(保留所有属性)
           "--delete", // 删除目标中多余的文件
           source,
           dest
       });
    }else{
       qDebug()<<" ##it's not directory## ";
       rsync.setArguments({
           "-aHAXog",       // 归档模式(保留所有属性)
           "--force",
           source,
           dest
       });
    }

    rsync.start();
    if (!rsync.waitForFinished(-1)) { // 60秒超时
       qCritical() << "Rsync process timeout:" << rsync.errorString();
       return false;
    }

    if (rsync.exitCode() != 0) {
       qCritical() << "Rsync copy failed: " << rsync.readAllStandardError() << "src "<< source << " dest "<<dest;
       return false;
    }
    return true;
}

void DataRestoreProxy::updateProgress(int processed)
{
    int percent = static_cast<int>((processed * 100.0) / m_totalItems);
    emit progress(percent);
}

void DataRestoreProxy::clearDirectory(const QString &dirPath)
{
    QDir dir(dirPath);

    // 如果目录不存在
    if (!dir.exists()) {
        return;
    }

    // 使用系统命令快速清空目录（包括隐藏文件）
    int ret = QProcess::execute("bash", {
        "-c",
        QString("shopt -s dotglob && rm -rf %1/*").arg(dirPath)
    });

    if (ret != 0) {
        qWarning() << "Failed to clear directory (command exited with" << ret << ")";
        return;
    }

    // 验证目录是否为空（排除 . 和 ..）
    bool isEmpty = dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
    if (!isEmpty) {
        qWarning() << "Directory is not empty after clearing:" << dirPath;
    }
    return;
}

QString DataRestoreProxy::generate16DigitRandomNumber()
{
    QString randomStr;
    for (int i = 0; i < 16; ++i) {
        // 生成 0-9 的随机数字
        int digit = QRandomGenerator::global()->bounded(10); // [0, 9]
        randomStr.append(QString::number(digit));
    }
    return randomStr;
}

int DataRestoreProxy::linkFile(QString src, QString desc)
{
    // 验证路径是否存在
    QFileInfo fileInfo(src);

    //如果是软连接，刚导出所指路径一定时有问题的，需要忽略
    if (!fileInfo.exists() && !fileInfo.isSymLink()) {
        qWarning() << "Source file does not exist:" << src;
        return -1;
    }

    //如果目标文件不是/data开头，需要补齐，否则无法实现硬链接
    if(!desc.startsWith("/data/"))
    {
        QString prePath = "/data/";
        desc = prePath + desc;
    }

    // 构建ln命令
    QStringList args;
    args << src << desc;

    QProcess m_process;
    m_process.start("ln", args);

    qDebug() << "硬链接地址：" << desc;

    if (!m_process.waitForStarted()) {
        qCritical() << "Failed to start rsync process";
        return -1;
    }

    if (!m_process.waitForFinished(-1)) {
        qCritical() << "rsync process failed to finish";
        return -1;
    }

    return m_process.exitCode();
}
