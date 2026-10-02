/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "utils.h"
#include <QByteArray>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QRegularExpression>
#include <QThread>
#include <QUuid>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <memory>
#include <functional>
#include <QRegularExpression>
#include <QProcess>
#include <QSettings>
#include <QTextCodec>
#include <QEventLoop>
#include <QTimer>
#include <QStorageInfo>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include "../backup-daemon/parsebackuplist.h"

#include "mylittleparse.h"
#include "mydefine.h"

QString SystemInfo::m_os;
QString SystemInfo::m_arch;
QString SystemInfo::m_archDetect;

QString Utils::m_sysRootPath = "/";
QString Utils::m_logPath = "";

QString Utils::m_backupRootPath = "/data/";

QDBusUnixFileDescriptor Utils::m_inhibitFileDescriptor = QDBusUnixFileDescriptor();

/**
 * @brief initSysRootPath, 根据应用程序路径推断系统根目录
 * @note
 *      本方法依赖于根分区挂载目录
 *      1. grub引导中根目录为/root（现在grub菜单的系统备份还原改为了initrd阶段用shell脚本实现，不会再用此应用程序）
 *      2. 试安装的小系统中根目录约定为/target（试安装小系统中进行系统还原的功能暂未提供）
 *      3. 正常使用中根目录为/
 *      此接口是为试安装的小系统中进行系统还原预留
 */
void Utils::initSysRootPath()
{
    QString sysRootPath = "/";
    if (Utils::isCasper())
        sysRootPath = CASPER_ROOT_PATH;

    m_sysRootPath = sysRootPath;
}

/**
 * @brief /proc/cmdline中包含boot=casper则说明是运行的试安装的小系统
 * @return true-是试安装模式中；false-不是
 */
bool Utils::isCasper()
{
    QString cmdline = Utils::processCmd("cat /proc/cmdline");
    if (cmdline.contains("boot=casper"))
        return true;
    else
        return false;
}

/**
 * @brief customMessageHandler，日志重定向句柄
 * @param type 日志类型，debug等
 * @param context 上下文
 * @param msg 日志信息
 */
void Utils::customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString strMsg("");
    switch (type) {
    case QtDebugMsg:
        strMsg = QString("[Debug]");
        break;
    case QtWarningMsg:
        strMsg = QString("[Warning]");
        break;
    case QtCriticalMsg:
        strMsg = QString("[Critical]");
        break;
    case QtFatalMsg:
        strMsg = QString("[Fatal]");
        break;
    default:
        strMsg = QString("[Debug]");
        break;
    }

    // 设置输出信息格式
    QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString strMessage = strMsg + QString("DateTime:%1 ThreadId:%2 Message:%3 File:%4(%5)")
            .arg(strDateTime).arg(QString::number(quintptr(QThread::currentThreadId()))).arg(localMsg.constData()).arg(context.file).arg(context.line);

    std::cout << strMessage.toUtf8().data() << std::endl;

    // 输出信息至文件中（读写、追加形式）
    QString fileName = m_sysRootPath + PROC_LOG;
    if (!m_logPath.isEmpty()) {
        fileName = m_logPath;
    }
    fileName.replace("//", "/");
    QFile file(fileName);
    file.open(QIODevice::ReadWrite | QIODevice::Append);
    QTextStream stream(&file);
    stream << strMessage << END_LINE;
    stream.flush();
    file.close();
}

/**
 * @brief 文件锁，锁定应用程序
 * @param frontUid，锁定程序的用户id
 * @return 锁文件的文件描述符
 * @note 使用方式类似于QLockFile，后面时间充足时也可以自行封装一个独立类来处理文件锁
 *       这个文件锁的作用是，在关机时关机模块会自行校验/tmp/lock路径下的锁文件，如果存在则提醒用户xx程序在运行不允许关机等
 */
int Utils::lockProgram(int frontUid)
{
    QDir dir(LOCK_FILE_PATH);
    if (!dir.exists()) {
        dir.mkdir(LOCK_FILE_PATH);
        chmod(LOCK_FILE_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
    }

    int lock_file_fd = ::open(LOCK_FILE, O_CREAT | O_RDWR, 0666);
    if (0 > lock_file_fd) {
        return -2;
    }
    fchmod(lock_file_fd, S_IRWXU | S_IRWXG | S_IRWXO);

    int lock_ret = flock(lock_file_fd, LOCK_EX | LOCK_NB);
    if (0 > lock_ret) {
        return -11;
    }

    ftruncate(lock_file_fd, 0);

    char write_string[PID_STRING_LEN] = { 0 };
    snprintf(write_string, PID_STRING_LEN, "%d\n%s\n", frontUid, BACKUP_CLI_NAME);
    write(lock_file_fd, write_string, strlen(write_string));
    fdatasync(lock_file_fd);
    return lock_file_fd;
}

/**
 * @brief 解除应用程序文件锁
 * @param lock_file_fd 锁文件的文件描述符
 * @return 0，解除成功；1，解除失败
 */
int Utils::unLockProgram(int lock_file_fd)
{
    int lock_ret = flock(lock_file_fd, LOCK_UN);
    if (lock_ret < 0) {
        qCritical("unlock fail!");
        return 1;
    }
    qDebug("unlock success!");
    rmLockFile();
    return 0;
}

/**
 * @brief 删除锁文件
 * @return bool
 */
bool Utils::rmLockFile()
{
    bool res = QFile::remove(LOCK_FILE);
    if (!res)
        qCritical() << QString("remove %s fail").arg(LOCK_FILE);
    return res;
}

/**
 * @brief 检查/etc/.bootinfo是否存在并可读，里面存放备份分区的UUID等信息
 * @return bool
 */
bool Utils::checkBootInfoExists()
{
    QString bootinfoPath = Utils::m_sysRootPath + BOOTINFO_PATH;
    bootinfoPath.replace("//", "/");
    QFile bootinfoFile(bootinfoPath);
    if (!bootinfoFile.exists()) {
        qCritical("%s is not exists!", qUtf8Printable(bootinfoPath));
        return false;
    }
    if (!bootinfoFile.open(QIODevice::ReadOnly)) {
        qCritical("%s file can't open!", qUtf8Printable(bootinfoPath));
        return false;
    }
    bootinfoFile.close();
    return true;
}

/**
 * @brief 是否有平板模式
 * @return bool
 */
bool Utils::isTablet()
{
    QString otaPath = Utils::m_sysRootPath + "/etc/apt/ota_version";
    otaPath.replace("//", "/");

    return QFile::exists(otaPath);
}

/**
 * @brief 获取备份分区的UUID
 * @return 备份分区的UUID
 */
QString Utils::getBackupPartitionUuid()
{
    QString bootinfoPath = Utils::m_sysRootPath + BOOTINFO_PATH;
    bootinfoPath.replace("//", "/");

    MyLittleParse parse(bootinfoPath);
    QString restoreUuid;
    parse.find("RECOVERY_DEV_UUID", restoreUuid);

    if (restoreUuid.isEmpty()) {
        QString fstab = Utils::m_sysRootPath + FSTAB_PATH;
        fstab.replace("//", "/");
        QFile file(fstab);
        if (file.exists()) {
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QString preLine;
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    if (line.isEmpty())
                        continue;
                    if (line.startsWith("#")) {
                        preLine = line;
                        continue;
                    }
                    if (line.startsWith("UUID=") && line.contains("/backup")) {
                        // like:
                        // # /dev/add4 LABEL=KYLIN-BACKUP
                        // UUID=40be1cac-cd92-49db-8a98-68ee21ddbc49  /backup ext4 noauto  0  0
                        int indexOfSpace = line.indexOf(QRegularExpression("[ \t]+"), 0);
                        QString uuid = line.mid(0, indexOfSpace);
                        uuid.replace("UUID=", "");
                        restoreUuid = uuid.trimmed();

                        parse.Add("RECOVERY_DEV_UUID", restoreUuid);

                        break ;
                    } else if (line.startsWith("/dev/") && line.contains("/backup")) {
                        // like:
                        // # UUID=40be1cac-cd92-49db-8a98-68ee21ddbc49 LABEL=KYLIN-BACKUP
                        // /dev/add4  /backup ext4 noauto  0  0
                        if (preLine.startsWith("#") && preLine.contains("UUID=")) {
                            preLine.replace("# ", "");
                            preLine.replace("#", "");

                            int indexOfSpace = preLine.indexOf(QRegularExpression("[ \t]+"), 0);
                            QString uuid = preLine.mid(0, indexOfSpace);
                            uuid.replace("UUID=", "");
                            restoreUuid = uuid.trimmed();

                            parse.Add("RECOVERY_DEV_UUID", restoreUuid);

                            break ;
                        }
                    }
                }
            }
        }
    }

    return restoreUuid;
}

/**
 * @brief 获取分区对应的UUID映射关系
 * @param fstab文件路径
 * @return “分区挂载目录-分区UUID”键值对
 */
QHash<QString, QString> Utils::getPartUuidMap(const QString &fstab)
{
    QHash<QString, QString> result;
    QFile file(fstab);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString preLine;
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.isEmpty())
                    continue;
                if (line.startsWith("#")) {
                    preLine = line;
                    continue;
                }

                // 配置文件/etc/fstab每行6个域: <file system> <mount point>   <type>  <options>       <dump>  <pass>, 形如:
                // UUID=232f5fb4-53e0-46b9-ba9b-22bfec64f2a2	/boot     	ext4      	rw,relatime	0 0
                QStringList list = line.split(QRegularExpression("[ \t]+"));
                QStringList fields;
                for (int i = 0; i < list.size(); ++i) {
                    QString field = list.at(i);
                    field = field.trimmed();
                    if (field.isEmpty())
                        continue;
                    fields << field;
                }
                QString mountPonit = fields.at(1);

                if (line.startsWith("UUID=")) {
                    // like:
                    // # /dev/add4 LABEL=KYLIN-BACKUP
                    // UUID=40be1cac-cd92-49db-8a98-68ee21ddbc49  /backup ext4 noauto  0  0
                    int indexOfSpace = line.indexOf(QRegularExpression("[ \t]+"), 0);
                    QString uuid = line.mid(0, indexOfSpace);
                    uuid.replace("UUID=", "");
                    uuid = uuid.trimmed();

                    result.insert(mountPonit, uuid);
                } else if (line.startsWith("/dev/")) {
                    // like:
                    // # UUID=40be1cac-cd92-49db-8a98-68ee21ddbc49 LABEL=KYLIN-BACKUP
                    // /dev/add4  /backup ext4 noauto  0  0
                    if (preLine.startsWith("#") && preLine.contains("UUID=")) {
                        preLine.replace("# ", "");
                        preLine.replace("#", "");

                        int indexOfSpace = preLine.indexOf(QRegularExpression("[ \t]+"), 0);
                        QString uuid = preLine.mid(0, indexOfSpace);
                        uuid.replace("UUID=", "");
                        uuid = uuid.trimmed();

                        result.insert(mountPonit, uuid);
                    }
                }
            }
        }
    }

    return result;
}

/**
 * @brief swapKeyValue
 * @param hash
 * @return
 */
QHash<QString, QString> Utils::swapKeyValue(const QHash<QString, QString>& hash)
{
    QHash<QString, QString> result;
    for (QHash<QString, QString>::const_iterator it = hash.constBegin(); it != hash.constEnd(); ++it) {
        result.insert(it.value(), it.key());
    }

    return result;
}

/**
 * @brief /etc/fstab中的绑定映射关系
 * @return src-dest键值对map
 */
QHash<QString, QString> Utils::getFstabBindInfo()
{
    QHash<QString, QString> result;
    QString fstabPath = Utils::m_sysRootPath + FSTAB_PATH;
    fstabPath.replace("//", "/");

    QFile file(fstabPath);
    if (!file.open(QIODevice::ReadOnly))
        return result;
    QTextStream in(&file);
    while (!in.atEnd()) {
       const QString &line = in.readLine();
       if (line.startsWith("#"))
           continue ;
       if (line.startsWith("UUID="))
           continue ;
       if (line.startsWith("/dev/"))
           continue ;
       if (line.isEmpty())
           continue ;
       if (!line.contains("bind"))
           continue ;

       // 配置文件/etc/fstab每行6个域: <file system> <mount point>   <type>  <options>       <dump>  <pass>, 形如:
       // UUID=232f5fb4-53e0-46b9-ba9b-22bfec64f2a2	/boot     	ext4      	rw,relatime	0 0
       QStringList list = line.split(QRegularExpression("[ \t]+"));
       QStringList fields;
       for (int i = 0; i < list.size(); ++i) {
          QString field = list.at(i);
          field = field.trimmed();
          if (field.isEmpty())
              continue;
          fields << field;
       }
       // 配置文件/etc/fstab每行6个域，第二个域为挂载路径, 为统一路径使用挂接点，排除第一个域
       if (6 == fields.size())
           result.insert(fields.at(0), fields.at(1));
    }

    file.close();
    return result;
}

/**
 * @brief 路径不存在则创建，不支持递归创建
 * @param path
 */
void Utils::mkdir(const QString& path)
{
    QDir dir(path);
    if (!dir.exists())
        dir.mkdir(path);
}

/**
 * @brief 创建路径，支持递归创建
 * @param path
 * @return bool
 */
bool Utils::mkpath(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return dir.mkpath(path);
    }
    return true;
}

/**
 * @brief 备份还原时，需要排除绑定挂载的路径，因为数据不是存放在挂载路径内
 * @param excludes，存放需要排除的绑定路径
 */
void Utils::excludeFstabBindPath(QStringList &excludes)
{
    QString fstabPath = Utils::m_sysRootPath + FSTAB_PATH;
    fstabPath.replace("//", "/");

    QFile file(fstabPath);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QTextStream in(&file);
    while (!in.atEnd()) {
       const QString &line = in.readLine();
       if (line.startsWith("#"))
           continue ;
       if (line.startsWith("UUID="))
           continue ;
       if (line.startsWith("/dev/"))
           continue ;
       if (line.isEmpty())
           continue ;
       if (!line.contains("bind"))
           continue ;

       // 配置文件/etc/fstab每行6个域: <file system> <mount point>   <type>  <options>       <dump>  <pass>, 形如:
       // UUID=232f5fb4-53e0-46b9-ba9b-22bfec64f2a2	/boot     	ext4      	rw,relatime	0 0
       QStringList list = line.split(QRegularExpression("[ \t]+"));
       QStringList fields;
       for (int i = 0; i < list.size(); ++i) {
          QString field = list.at(i);
          field = field.trimmed();
          if (field.isEmpty())
              continue;
          fields << field;
       }
       // 配置文件/etc/fstab每行6个域，第二个域为挂载路径, 为统一路径使用挂接点，排除第一个域
       if (6 == fields.size())
           excludes << fields.at(0);
    }

    file.close();
}

/**
 * @brief 备份还原时，需要一些用户家目录下的路径，因为这些路径备份还原程序无权操作
 * @param excludes，存放需要排除的路径
 */
void Utils::excludeSomeHomePath(QStringList &excludes)
{
    QSet<QString> homeSet;
    // 获取/home下的用户家目录
    QString homePath = Utils::m_sysRootPath + "/home";
    homePath.replace("//", "/");
    QDir dir(homePath);
    QStringList list = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (const QString& item : list) {
        // /home/oem跳过
        if (item == "oem")
            continue ;

        QString home("/home/");
        home += item;
        excludes << home + "/.box";
        excludes << home + "/.log";
        QString dataHome("/data/home/");
        dataHome += item;
        excludes << dataHome + "/.box";
        excludes << dataHome + "/.log";
    }
}

/**
 * @brief 获取指定目录的子目录列表
 * @param parent，指定的父目录
 */
QStringList Utils::getSubDirs(const QString& parent)
{
    QStringList subs;
    // 获取home下的用户家目录
    QDir dir(parent);
    QStringList subDirs = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (const QString& subDir : subDirs) {
        subs << subDir;
    }

    return subs;
}

/**
 * @brief 获取额外备份或还原数据
 * @param extraPaths，存放额外需要备份或还原的数据路径列表
 * @note 主要应用于系统更新备份/还原、保留用户数据还原，需要备份\还原这些额外数据；否则，可能引起某些软件的用户配置和软件版本不匹配造成的不能正常使用
 */
void Utils::extraBackupOrRestorePaths(QStringList &extraPaths)
{
    QString conf = Utils::getSysRootPath() + EXTRA_BACKUP_OR_RESTORE_CONF;
    conf.replace("//", "/");
    QFile file(conf);
    QString line;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        while(!stream.atEnd()){
             line = stream.readLine();
             line = line.trimmed();
             extraPaths << line;
        }

        file.close();
    }
}

/**
 * @brief 排除自定义备份路径
 * @param excludes，存放需要排除的路径
 */
void Utils::excludeCustomizePath(QStringList &excludes)
{
    // 本地xml文件中的信息
    QString xmlPath = Utils::getSysRootPath() + SYS_UPDATE_BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    parse.getCustomizePaths(excludes);

    xmlPath = Utils::getSysRootPath() + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList xmlparse(xmlPath);
    xmlparse.getCustomizePaths(excludes);
}

/**
 * @brief 生成rsync --exclude-from排除路径规则文件
 * @return
 */
bool Utils::generateExcludePathsFile()
{
    QString excludeFile = Utils::m_sysRootPath + EXCLUDE_FILE_PATH;
    excludeFile.replace("//", "/");

    QFile excludePathFile(excludeFile);
    // 暂时改为每次都重写文件内容，以便能随版本更新排除路径
    if (!excludePathFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical("%s create failed", qUtf8Printable(excludeFile));
        return false;
    }

    QTextStream in(&excludePathFile);
    in << "/backup" << END_LINE; //分区
    // in << "/boot/efi" << END_LINE;
    in << "/cdrom" << END_LINE;
    in << "/dev" << END_LINE;
    // efi原始目录在/boot/efi，备份到目标目录为/efi下，再还原时已经单独处理了，批量还原时应该屏蔽此目录
    in << "/efi" << END_LINE;
    // 安全模块会将文件/usr/share/kysec-utils/data/readonly_list中的文件列表限制只读，无法修改、备份（包含扩展属性时）、删除等
    // 现在里面仅有/etc/uid_list，先暂时排除掉；等后续安全模块有其它保护方案后再进一步修改
    in << "/etc/uid_list" << END_LINE;
    in << "/data/ghost" << END_LINE; //ghost镜像文件
    in << "/ghost" << END_LINE; //ghost镜像文件
    in << "/lost+found" << END_LINE;
    in << "/media" << END_LINE;
    in << "/mnt" << END_LINE;
    in << "/proc" << END_LINE;
    in << "/run" << END_LINE;
    in << "/swap_file" << END_LINE;
    in << "/sys" << END_LINE; //添加*(/sys/*)，表示如果/sys目录不存在，则会拷贝/sys，但不会拷贝/sys下的内容
    in << "/tmp" << END_LINE;
    in << "/var/lib/docker/overlay2" << END_LINE;
    // 安卓兼容的这个里面很多文件都是设置了特殊扩展文件属性，lsetxattr无法设置成功，听取安卓兼容模块同事的意见不用管这个文件夹，其实是从home下挂载的
    in << "/var/lib/kmre/data" << END_LINE;
    in << "/var/lib/kmre/kmre-*-*/data/media/0/0-麒麟*" << END_LINE;
    in << "/var/lib/udisks2" << END_LINE;
    in << "/var/log" << END_LINE;
    in << "*/backup/snapshots" << END_LINE;
    // 跟wps的研发沟通了，安全目录的使用场景，主要是两个场景：
    //    1、在用户浏览信创加密文档时，解密出来的临时明文文件会放在安全目录，关闭文档后，临时文件被删除。
    //    2、用户离线授权的某个加密文档的解密私钥也会放在安全目录中。因此跟他讨论，从安全角度来看，最好不要备份还原这个目录。
    in << "/data/security-dir" << END_LINE;

    // 系统安装后有的会将/data/home /data/root挂载到的/home /root上，实际文件是存放在/data/home /data/root下面，为了统一标准保留/home /root排除/data/home /data/root
    QStringList excludes;
    Utils::excludeFstabBindPath(excludes);
    // ~/.box目录后来安全给予赋权，故此去掉
    // Utils::excludeSomeHomePath(excludes);
    // 已经包含在"*/backup/snapshots"中
    // Utils::excludeCustomizePath(excludes);
    for (const QString& item : excludes) {
        in << item << END_LINE;
    }
    in.flush();

    excludePathFile.close();
    return true;
}

/**
 * @brief 判断目录是否存在
 * @param 目录路径
 * @return true,存在；false,不存在
 */
bool Utils::isDirExist(const QString& fullDirName)
{
    QDir dir(fullDirName);
    if (dir.exists())
        return true;
    return false;
}

/**
 * @brief 获取系统备份还原排除路径列表, rsync --exclude-from排除路径规则文件中读取
 * @return 系统备份还原排除路径列表
 */
QStringList Utils::getFromExcludePathsFile()
{
        QStringList list;
        list << "/fs.old*";
        list << "/backup";
        // list << "/boot/efi";
        list << "/cdrom";
        list << "/dev";
        list << "/efi";
        list << "/etc/uid_list";
        list << "/data/ghost";
        list << "/ghost";
        list << "/lost+found";
        list << "/media";
        list << "/mnt";
        list << "/proc";
        list << "/run";
        list << "/swap_file";
        list << "/sys";
        list << "/tmp";
        list << "/var/lib/docker/overlay2";
        list << "/var/lib/kmre/data";
        list << "/var/lib/kmre/kmre-*-*/data/media/0/0-麒麟*";
        list << "/var/lib/udisks2";
        list << "/var/log";
        list << "*/backup/snapshots";
        // 跟wps的研发沟通了，安全目录的使用场景，主要是两个场景：
        //    1、在用户浏览信创加密文档时，解密出来的临时明文文件会放在安全目录，关闭文档后，临时文件被删除。
        //    2、用户离线授权的某个加密文档的解密私钥也会放在安全目录中。因此跟他讨论，从安全角度来看，最好不要备份还原这个目录。
        list << "/data/security-dir";

        // 系统安装后有的会将/data/home /data/root挂载到的/home /root上，实际文件是存放在/data/home /data/root下面
        QStringList excludes;
        Utils::excludeFstabBindPath(excludes);
        // Utils::excludeSomeHomePath(excludes);
        // 已经包含在"*/backup/snapshots"中
        // Utils::excludeCustomizePath(excludes);
        for (const QString& item : excludes) {
            list << item;
        }

        return list;
}

/**
 * @brief 生成Uuid
 * @return UUID
 */
QString Utils::createUuid()
{
    QString uuid;
    do {
        uuid = QUuid::createUuid().toString();
    } while (uuid == AUTO_BACKUP_UUID || uuid == FACTORY_BACKUP_UUID);
    return uuid;
}

/**
 * @brief 将列表中内容写入指定文件中
 * @param fileName，文件全路径
 * @param lines，内容列表
 * @return true，成功写入；false，写入失败
 */
bool Utils::writeFileByLines(const QString& fileName, const QStringList& lines)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    for (const QString& line : lines) {
        out << line << END_LINE;
    }
    out.flush();
    file.close();
    return true;
}

/**
 * @brief 判断文件是否存在
 * @param fileName 文件明
 * @return bool，true-存在；false-不存在
 * @author zhaominyong
 * @since 2021/05/29
 */
bool Utils::fileExists(const QString &fileName)
{
    struct stat buffer;
    return (stat(fileName.toStdString().data(), &buffer) == 0);
}

/**
 * @brief 判断目录是否为空
 * @param fullDirName 目录路径
 * @return true-目录不存在或为空目录; false-非空目录
 */
bool Utils::isDirEmpty(const QString& fullDirName)
{
    QDir dir(fullDirName);
    if (!dir.exists())
        return true;

    if (dir.isEmpty())
        return true;

    return false;
}

/**
 * @brief 记录备份日志
 * @param line，日志内容
 * @return bool, true-成功；false-失败
 * @author zhaominyong
 * @since 2021/05/29
 * @note
 *      因为系统恢复成功后马上需要reboot，但是此时的文件缓存可能还未能落盘，故此增加此函数，其中调用了系统函数fdatasync确保缓存落盘
 */
bool Utils::writeBackupLog(QString line)
{
    line = line + "\n";
    QString logFile = Utils::getSysRootPath() + BACKUP_LOG_TEXT_PATH;
    logFile.replace("//", "/");
    std::string fileName(logFile.toStdString());
    // 判断文件是否存在
    bool exists = fileExists(logFile);

    std::unique_ptr<std::FILE, int (*)(std::FILE*)> fp(std::fopen(fileName.data(), "a+"), std::fclose);

    if (!fp) {
        std::perror("file opening failed!");
        return false;
    }

    if (!exists) {
        std::fputs("#This file must not be deleted and it is for the backup tool.\n", fp.get());
        std::fputs("#0:new system backup\n", fp.get());
        std::fputs("#1:inc system backup\n", fp.get());
        std::fputs("#2:new data backup\n", fp.get());
        std::fputs("#3:inc data backup\n", fp.get());
        std::fputs("#4:restore system\n", fp.get());
        std::fputs("#5:retaining user data\n", fp.get());
        std::fputs("#6:restore data\n", fp.get());
        std::fputs("#7:inc restore data\n", fp.get());
        std::fputs("#8:delete backup\n", fp.get());
        std::fputs("#9:ghost image\n", fp.get());
        std::fputs("#9:auto backup system\n", fp.get());
        std::fputs("#for example: 17-04-25 10:43:56,{uuidxxxxx},0,this is a note,21.19KB,userid-1000,备份名称\n", fp.get());
    }
    std::fputs(line.toStdString().data(), fp.get());
    std::fflush(fp.get());
    fdatasync(fileno(fp.get()));

    return true;
}

/**
 * @brief 立即写文件
 * @param fileName 文件名，包含路径
 * @param content 文件内容
 * @return bool
 */
bool Utils::syncWriteFile(const QString &fileName, const QString& content)
{
    qDebug() << fileName << content;
    std::unique_ptr<std::FILE, int (*)(std::FILE*)> fp(std::fopen(fileName.toStdString().data(), "w"), std::fclose);

    if (!fp) {
        std::perror("file opening failed!");
        return false;
    }

    QStringList lines = content.split("\n");
    for (const QString& line : lines) {
        std::fputs(line.toStdString().data(), fp.get());
    }
    std::fflush(fp.get());
    fdatasync(fileno(fp.get()));

    return true;
}

/**
 * @brief 将字节大小转换为GB等表示的字符串
 * @param size，qint64，空间大小，单位字节
 * @return GB/MB/KB等表示的字符串型大小
 */
QString Utils::StringBySize(qint64 size)
{
#define GB (1024 * 1024 * 1024)
#define MB (1024 * 1024)
#define KB (1024)

    float fcount = size * 1.0;

    if (size > GB)
        return QString(QString::number(fcount / GB, 10, 2) + "GB");
    else if (size > MB)
        return QString(QString::number(fcount / MB, 10, 2) + "MB");
    else
        return QString(QString::number(fcount / KB, 10, 2) + "KB");
}

/**
 * @brief 获取挂接的移动设备列表
 * @return MOUNTPOINT,PATH键值对列表
 * @author zhaominyong
 * @since 2021/06/07
 * @note
 *      for bug  59636 【cpm】【HUAWEI】【KOS】【L0】【V3试制】备份时选择移动设备，会出现一个dev/sdc的路径(一般+必现+不常用功能)
 *      QStorageInfo::mountedVolumes获取的磁盘列表区分不出来是否移动设备
 */
QHash<QString, QString> Utils::getRemovableStorages()
{
    QHash<QString, QString> removalbeStorages;

    QProcess process_lsblk;
    process_lsblk.start("lsblk -P -o PATH,RM,TYPE,MOUNTPOINT,TRAN");
    if (!process_lsblk.waitForStarted(-1)) {
        return removalbeStorages;
    }
    if (!process_lsblk.waitForFinished(-1)) {
        return removalbeStorages;
    }
    QString result = process_lsblk.readAllStandardOutput();
    /*
    result like bellow :
    PATH="/dev/sda" RM="0" TYPE="disk" MOUNTPOINT="" TRAN="sata"
    PATH="/dev/sda1" RM="0" TYPE="part" MOUNTPOINT="/media/zhaominyong/DATA1" TRAN=""
    PATH="/dev/sdb" RM="1" TYPE="disk" MOUNTPOINT=""  TRAN="usb"
    PATH="/dev/sdb4" RM="1" TYPE="part" MOUNTPOINT="/media/zhaominyong/31  GB"  TRAN=""
    PATH="/dev/sr0" RM="1" TYPE="rom" MOUNTPOINT="/media/zhaominyong/Kylin-Desktop-V10-SP1"  TRAN=""
    */
    QStringList items = result.split("\n");
    QStringList usbDevs;
    for (QStringList::const_iterator it = items.constBegin(); it != items.constEnd(); ++it) {
        const QString &line = (*it);
        if (line.contains(" TRAN=\"usb\"")) {
            QStringList storageAttrs = line.split("\" ");
            if (5 != storageAttrs.size())
                continue ;
            QString path = storageAttrs.at(0);
            path = path.replace("PATH=\"", "");
            usbDevs.append(path);
        }
        if (line.contains(" RM=\"1\" ") || line.contains(" MOUNTPOINT=\"/media/") || line.contains(" MOUNTPOINT=\"/run/media/")) {
            if (line.contains(" TYPE=\"rom\" "))
                continue;
            if (line.contains(" MOUNTPOINT=\"\""))
                continue;

            // "PATH RM TYPE MOUNTPOINT TRAN" for each row, split by "\" "
            QStringList storageAttrs = line.split("\" ");
            if (5 != storageAttrs.size())
                continue ;
            QString path = storageAttrs.at(0);
            path = path.replace("PATH=\"", "");
            bool isSubPart = false;
            for (const QString& usbDev : usbDevs) {
                if (path.contains(usbDev)) {
                    isSubPart = true;
                    break;
                }
            }
            if (!isSubPart)
                continue;
            QString mount_point = storageAttrs.at(3);
            mount_point = mount_point.replace("MOUNTPOINT=\"", "");
            removalbeStorages.insert(path, mount_point);
        }
    }

    return removalbeStorages;
}

/**
 * @brief 获取分区信息
 * @return “LABEL-分区信息”键值对
 */
QHash<QString, PartitionInfo> Utils::getPartitions()
{
    QString blkidResult = Utils::processCmd("blkid");
    QStringList lines = blkidResult.split("\n");
    /*
    blkid命令结果形如：
    /dev/sda3: LABEL="SYSROOT" UUID="0336d6bf-4ce5-4dcf-8976-4f51be65e0ea" TYPE="ext4" PARTLABEL="SYSROOT" PARTUUID="d255022a-597e-471a-8407-bf8f82c66f8c"
    /dev/sda1: LABEL_FATBOOT="ESP" LABEL="ESP" UUID="A995-1AE9" TYPE="vfat" PARTLABEL="EFI" PARTUUID="98282378-7d21-4ee8-96f2-abbe9903b6db"
    /dev/sda2: LABEL="SYSBOOT" UUID="939dc93e-6c81-4f50-8de8-abd8e5ad2058" TYPE="ext4" PARTLABEL="boot" PARTUUID="b3638559-0ebd-4df1-96ef-49058e703a7f"
    /dev/sda4: LABEL="KYLIN-BACKUP" UUID="5f98378a-56fc-4ba5-a751-2bc51c59a0a4" TYPE="ext4" PARTLABEL="backup" PARTUUID="31a2e37b-2868-4959-8402-23e135ddd9aa"
    /dev/sda5: LABEL="DATA" UUID="f363d368-ef4e-4a19-a997-be4c3bfaaca5" TYPE="ext4" PARTLABEL="data" PARTUUID="c24d9d39-9cb1-4eb2-9b88-e807decbb8e5"
    /dev/sda6: LABEL="SWAP" UUID="577b58ad-78a4-4a68-84b6-07095d562614" TYPE="swap" PARTLABEL="logical" PARTUUID="acba62b4-0747-44d9-9d87-82013762af31"
     */

    QHash<QString, PartitionInfo> result;
    for (const QString& line : lines) {
        QStringList fields = line.split(QRegularExpression(":[ \t]+"));
        if (fields.size() != 2)
            continue;
        QString device = fields.at(0);
        QString desc = fields.at(1);
        PartitionInfo partitionInfo;
        partitionInfo.m_device = device;
        QStringList attrs = desc.split(QRegularExpression("[ \t]+"));
        for (const QString& attr : attrs) {
            if (attr.startsWith("LABEL=")) {
                QStringList pairs = attr.split("=");
                if (pairs.size() != 2)
                    continue;
                QString label = pairs.at(1);
                label.replace("\"", "");
                partitionInfo.m_label = label;
            } else if (attr.startsWith("UUID=")) {
                QStringList pairs = attr.split("=");
                if (pairs.size() != 2)
                    continue;
                QString uuid = pairs.at(1);
                uuid.replace("\"", "");
                partitionInfo.m_uuid = uuid;
            } else if (attr.startsWith("TYPE=")) {
                QStringList pairs = attr.split("=");
                if (pairs.size() != 2)
                    continue;
                QString type = pairs.at(1);
                type.replace("\"", "");
                partitionInfo.m_type = type;
            }

            if (!partitionInfo.m_label.isEmpty() && !partitionInfo.m_uuid.isEmpty() && !partitionInfo.m_type.isEmpty()) {
                result.insert(partitionInfo.m_label, partitionInfo);
                break;
            }
        }
    }

    return result;
}

/**
 * @brief 获取挂接的计算机内部磁盘
 * @return 内部磁盘挂接路径列表
 */
QList<QString> Utils::getLocalDisks()
{
    QList<QString> localDisks;

    QProcess process_lsblk;
    process_lsblk.start("lsblk -P -o PATH,RM,TYPE,MOUNTPOINT,TRAN");
    if (!process_lsblk.waitForStarted(-1)) {
        return localDisks;
    }
    if (!process_lsblk.waitForFinished(-1)) {
        return localDisks;
    }
    QString result = process_lsblk.readAllStandardOutput();

    QString userName = qgetenv("USER");
    QString mountPointPre("/media/");
    mountPointPre += userName;
    /*
    result like bellow :
    PATH="/dev/sda" RM="0" TYPE="disk" MOUNTPOINT="" TRAN="sata"
    PATH="/dev/sda1" RM="0" TYPE="part" MOUNTPOINT="/media/zhaominyong/DATA1" TRAN=""
    PATH="/dev/sdb" RM="1" TYPE="disk" MOUNTPOINT=""  TRAN="usb"
    PATH="/dev/sdb4" RM="1" TYPE="part" MOUNTPOINT="/media/zhaominyong/31  GB"  TRAN=""
    PATH="/dev/sr0" RM="1" TYPE="rom" MOUNTPOINT="/media/zhaominyong/Kylin-Desktop-V10-SP1"  TRAN=""
    */
    QStringList items = result.split("\n");
    QStringList usbDevs;
    for (QStringList::const_iterator it = items.constBegin(); it != items.constEnd(); ++it) {
        const QString &line = (*it);
        if (line.contains(QRegularExpression(" TRAN=\".+\"")) && !line.contains(" TRAN=\"usb\"")) {
            QStringList storageAttrs = line.split("\" ");
            if (5 != storageAttrs.size())
                continue ;
            QString path = storageAttrs.at(0);
            path = path.replace("PATH=\"", "");
            usbDevs.append(path);
            continue;
        }
        if (line.contains(" TYPE=\"part\" ") && line.contains(mountPointPre)) {
            if (line.contains(" MOUNTPOINT=\"\""))
                continue;

            // "PATH RM TYPE MOUNTPOINT TRAN" for each row, split by "\" "
            QStringList storageAttrs = line.split("\" ");
            if (5 != storageAttrs.size())
                continue ;
            QString path = storageAttrs.at(0);
            path = path.replace("PATH=\"", "");
            bool isSubPart = false;
            for (const QString& usbDev : usbDevs) {
                if (path.contains(usbDev)) {
                    isSubPart = true;
                    break;
                }
            }
            if (!isSubPart)
                continue;

            // 默认挂载的内部磁盘不太可能使用中文字符等，暂不考虑挂载路径被转义的场景
            QString mount_point = storageAttrs.at(3);
            mount_point = mount_point.replace("MOUNTPOINT=\"", "");
            localDisks << mount_point;
        }
    }

    return localDisks;
}

/**
 * @brief 向文件里面覆盖写文本内容
 * @param fileName
 * @param text
 * @return true-成功; false-失败
 */
bool Utils::writeFile(const QString& fileName, const QString& text)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        qDebug() << QString("打开文件%1失败").arg(fileName);
        return false;
    }

    QTextStream out(&file);
    out << text;
    out.flush();
    file.close();
    return true;
}

/**
 * @brief 设置安全状态，后面会逐渐作废，改为引入安全包libkysec-extend-dev和libkysec-extend，使用kysec/status_dbus.h中的接口kysec_setstatus
 * @param flag
 */
void Utils::echoKysecStatus(bool flag)
{
    //也可以引入安全包libkysec-extend-dev和libkysec-extend，使用kysec/status_dbus.h中的接口kysec_setstatus
    QString msg;
    if (flag) {
        qDebug("echo 2 > /sys/kernel/security/kysec/status");
        Utils::executeCMD("echo 2 > /sys/kernel/security/kysec/status", msg);
    } else {
        Utils::executeCMD("echo 4 > /sys/kernel/security/kysec/status", msg);
    }
    Utils::executeCMD("getstatus", msg);
    qDebug() << "kysec status :" << msg;
}

/**
 * @brief 设置安全状态
 * @param enable——true，开启保护；false，关闭保护
 */
void Utils::setKysecStatus(bool enable)
{
    QFile file("/usr/bin/setstatus");
    if (!file.exists()) {
        file.setFileName("/usr/sbin/setstatus");
        if (!file.exists())
            return ;
    }

    if (enable) {
        // 2209-RC8.4中失效，2209-RC8.2中还好好的，安全的同事也没弄清楚原因
//        QProcess::execute("setstatus enable");
        QString msg;
        Utils::executeCMD("setstatus enable", msg);
        qDebug() << msg;
        // 设置保护模式: echo 2 > /sys/kernel/security/kysec/status
        // Utils::writeFile("/sys/kernel/security/kysec/status", "2");
//        echoKysecStatus(enable);
        Utils::executeCMD("getstatus", msg);
        qDebug() << msg;
    } else {
        // 2209-RC8.4中失效，2209-RC8.2中还好好的，安全的同事也没弄清楚原因
//        QProcess::execute("setstatus softmode");
        QString msg;
        Utils::executeCMD("setstatus softmode", msg);
        qDebug() << msg;
        // 设置softmode模式: echo 4 > /sys/kernel/security/kysec/status
        // Utils::writeFile("/sys/kernel/security/kysec/status", "4");
//        echoKysecStatus(enable);
        Utils::executeCMD("getstatus", msg);
        qDebug() << msg;
    }
}

/**
 * @brief 启动或关闭kysec-sync-daemon服务
 * @param enable——true，开启；false，关闭
 */
void Utils::setKysecDaemon(bool enable){
    QString msg;
    if(enable){
        Utils::executeCMD("systemctl start kysec-sync-daemon", msg);
    }else{
        Utils::executeCMD("systemctl stop kysec-sync-daemon", msg);
    }
}

/**
 * @brief 判断程序是否已开启
 * @param processName
 * @return bool
 */
bool Utils::isRunning(const QString &processName)
{
    bool bRet = false;

    QProcess ps;
    QProcess grep;
    ps.setStandardOutputProcess(&grep);

    ps.start("ps", QStringList() << "-ef");
    grep.start("grep", QStringList() << processName);

    if (grep.waitForFinished()) {
        QString result(grep.readAll());
        qDebug() << result;
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

/**
 * @brief 判断是否990或9006C、PANGU处理器
 * @return bool
 */
bool Utils::isHuawei990()
{
    QString result;
    Utils::executeCMD("cat /proc/cpuinfo | grep -i \"kirin.*9.0\"", result);
    if (result.isEmpty()) {
        Utils::executeCMD("cat /proc/cpuinfo | grep -i \"PANGU\"", result);
        if (result.isEmpty())
            return false;
    }

    return true;
}

/**
 * @brief 在终端中执行简单命令
 * @param cmd
 * @return 返回终端输出结果
 */
QString Utils::processCmd(const QString& cmd)
{
    QString result;

    QStringList args;
    args << "-c";
    args << cmd;
    QProcess process;
    process.start("bash", args);
    if (process.waitForFinished()) {
        result = QString(process.readAll());
    }

    return result;
}

/**
 * @brief popen()函数通过创建一个管道，调用fork()产生一个子进程，执行一个shell以运行命令来开启一个进程。
 * @param cmd，是一个指向以 NULL 结束的 shell 命令字符串的指针。这行命令将被传到 bin/sh 并使用 -c 标志，shell 将执行这个命令
 * @param result 输出结果
 * @note
 */
void Utils::executeCMD(const char* cmd, QString &result)
{
    char buf_ps[1024] = { 0 };
    FILE* ptr = nullptr;
    // 只能是读或者写中的一种，得到的返回值（标准 I/O 流）也具有和 type 相应的只读或只写类型。
    // 如果 type 是 “r” 则文件指针连接到 command 的标准输出；如果 type 是 “w” 则文件指针连接到 command 的标准输入。
    qDebug() << cmd;
    if ((ptr = popen(cmd, "r")) != nullptr) {
        while (fgets(buf_ps, 1024, ptr) != nullptr) {
            result += buf_ps;
            memset(buf_ps, 0, 1024);
        }
        pclose(ptr);
        ptr = nullptr;
    } else {
        qCritical("popen %s error", cmd);
    }
}

/**
 * @brief Utils::executeCmd
 * @param cmd
 * @param args
 * @return
 */
QString Utils::executeCmd(const QString &cmd, const QStringList &args)
{
    QString cmdLine(cmd);
    for (const QString &arg : args) {
        cmdLine += " ";
        cmdLine += arg;
    }
    qDebug() << cmdLine;

    QString result;
    char buf_ps[1024] = { 0 };
    FILE* ptr = nullptr;
    // 只能是读或者写中的一种，得到的返回值（标准 I/O 流）也具有和 type 相应的只读或只写类型。
    // 如果 type 是 “r” 则文件指针连接到 command 的标准输出；如果 type 是 “w” 则文件指针连接到 command 的标准输入。
    if ((ptr = popen(cmdLine.toStdString().data(), "r")) != nullptr) {
        while (fgets(buf_ps, 1024, ptr) != nullptr) {
            result += buf_ps;
            memset(buf_ps, 0, 1024);
        }
        pclose(ptr);
        ptr = nullptr;
    } else {
        qCritical("popen %s error", cmdLine.toStdString().data());
    }
    return result;
}

/**
 * @brief 获取备份日志列表
 * @return
 */
QList<BackupWrapper> Utils::getBackupLogList()
{
    // 为兼容以前的老备份数据，在此处先获取uuid-name键值对
    QMap<QString, QString> uuid_name = Utils::getBackupUuidNameMap();

    QString logFile = Utils::getSysRootPath() + "/data" +  BACKUP_LOG_TEXT_PATH;
    logFile.replace("//", "/");
    QFile file(logFile);
    QList<BackupWrapper> list;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("#") || line.isEmpty())
                continue;
            QStringList fields = line.split(",");
            BackupWrapper record;
            int index = 0;
            for (const QString &field : fields) {
                switch (index) {
                case 0: // 备份时间
                    // 日志中记录的时间格式为：yy-MM-dd hh:mm:ss,转换为yyyy-MM-dd hh:mm:ss显示
                    record.m_time = field;
                    break;
                case 1: // uuid
                    record.m_uuid = field;
                    break;
                case 2: // 操作类型
                    record.m_type = field.toInt();
                    break;
                case 3: // 备注
                    record.m_note = field;
                    break;
                case 4: // 备份大小
                    record.m_size = field;
                    break;
                case 5: // 备份用户id
                    if (field.isEmpty())
                        record.m_frontUid = -1;
                    else
                        record.m_frontUid = field.toInt();
                    break;
                case 6: // 备份名称
                    record.m_backupName = field;
                    break;
                default:
                    break;
                }

                ++index;
            }

            if (record.m_backupName.isEmpty()) {
                if (FACTORY_BACKUP_UUID == record.m_uuid) {
                    record.m_backupName = QObject::tr("Factory Backup");
                }
//                else if (AUTO_BACKUP_UUID == record.m_uuid) {
//                    record.m_backupName = QObject::tr("Auto Backup");
//                }
                else if (uuid_name.contains(record.m_uuid)) {
                    record.m_backupName = uuid_name.value(record.m_uuid);
                }
                else {
                    record.m_backupName = record.m_time;
                }
            } else if (FACTORY_BACKUP_UUID == record.m_uuid) {
                record.m_backupName = QObject::tr("Factory Backup");
            }

            list << record;
        }
    }

    return list;
}

/**
 * @brief 初始化系统信息
 */
void Utils::initSystemInfo()
{
    SystemInfo::m_os = Utils::getOs();
    SystemInfo::m_arch = Utils::getArch();
    SystemInfo::m_archDetect = Utils::getArchDetect();
}

/**
 * @brief getOs
 * @return 操作系统名字, 如：
 *         Kylin-Desktop V10-SP1
 *         Build 20210407
 */
QString Utils::getOs()
{
    QString path = Utils::getSysRootPath() + "/etc/kylin-build";
    path.replace("//", "/");
    QFile file(path);
    /* 文件内容形如：
    Kylin-Desktop V10-SP1
    Build 20210407
    */
    QString os;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        if (!in.atEnd()) {
            os = in.readLine();
        }
    }

    return os;
}

/**
 * @brief getArch
 * @return 架构，arch命令的结果，如：x86_64
 */
QString Utils::getArch()
{
    QString arch = Utils::executeCmd("arch");
    return arch.replace("\n", "");
}

/**
 * @brief getArchDetect
 * @return archdetect命令的结果，如：amd64/efi
 */
QString Utils::getArchDetect()
{
    QString arch = Utils::executeCmd("archdetect");
    return arch.replace("\n", "");
}

/**
 * @brief 获取分区剩余大小
 * @return
 */
QMap<QString, QString> Utils::getLeftSizeOfPartitions()
{
    QString root = Utils::getSysRootPath();
    QString backup = Utils::getSysRootPath() + BACKUP_PATH;
    backup.replace("//", "/");
    QString data = Utils::getSysRootPath() + DATA_PATH;
    data.replace("//", "/");

    QStringList args;
    args << "-h";
    args << backup;
    args << data;
    args << root;

    QMap<QString, QString> map;

    QProcess df;
    df.start("df", args);
    if (df.waitForFinished()) {
        QString result(df.readAll());
        QStringList lines = result.split("\n");
/*
文件系统        容量  已用  可用 已用% 挂载点
/dev/nvme0n1p4   26G   45M   25G    1% /backup
/dev/nvme0n1p3   98G   54G   40G   58% /
/dev/nvme0n1p5   90G  8.6G   77G   11% /data
*/
        // 排除第一行标题
        for (int i = 1; i < lines.size(); ++i) {
            QString line = lines.at(i);
            line = line.trimmed();
            if (line.isEmpty())
                continue;

            QStringList fields = line.split(QRegularExpression("[ \t]+"));
            if (fields.size() != 6)
                continue;
            QString path = fields.at(5);
            QString left = fields.at(3);
            map.insert(path, left);
        }
    }
    return map;
}

/**
 * @brief 获取分区可用大小，主要是/和/data分区
 * @return 路径和大小（单位：字节）键值对
 */
QHash<QString, qint64> Utils::getAvailableSizeOfPartitions()
{
    QHash<QString, qint64> hash;

    QString root = Utils::getSysRootPath();
    QStorageInfo rootInfo(root);
    hash.insert(root, rootInfo.bytesAvailable());

    QString data = Utils::getSysRootPath() + DATA_PATH;
    data.replace("//", "/");
    QStorageInfo dataInfo(data);
    hash.insert(data, dataInfo.bytesAvailable());

    return hash;
}

/**
 * @brief 获取文件夹或文件的大小
 * @param path 路径
 * @return 大小
 */
qint64 Utils::getDirOrFileSize(const QString &path)
{
    // 匹配不包含特殊字符的字符串
    QRegularExpression regex("^[^<>`,;|#\\^\\$]+$");

    // 检查是否匹配
    if (regex.match(path).hasMatch()) {
        qDebug() << "字符串合法:" << path;
        char cmd[1024] = { 0 };
        sprintf(cmd, "du -sb '%s' 2>/dev/null | awk '{print $1}'", path.toLocal8Bit().data()); //s:total k:1024 bytes b:1 byte

        QString qsSize;
        Utils::executeCMD(cmd, qsSize);
        qsSize.replace("\n", "");
        qsSize = qsSize.trimmed();
        return qsSize.toLongLong();
    } else {
        qDebug() << "字符串包含非法字符:" << path;
        return 0;
    }
}

/**
 * @brief 记录下备份点uuid及其名称
 * @param uuid 备份点识别码
 * @param backupName 备份点名称
 */
void Utils::update_backup_unique_settings(const QString &uuid, const QString &backupName)
{
    // 由于历史原因（原来只校验U盘备份的唯一性）,文件名暂不改变
    QString backupUniqueSetting = Utils::getSysRootPath() + UDISK_UNIQUE_SETTINGS;
    backupUniqueSetting.replace("//", "/");
    QSettings udisk_unique_settings(backupUniqueSetting, QSettings::IniFormat);
    udisk_unique_settings.setIniCodec(QTextCodec::codecForName("utf-8"));
    udisk_unique_settings.beginGroup(backupName);
    udisk_unique_settings.setValue("uuid", uuid);
    udisk_unique_settings.endGroup();
    udisk_unique_settings.sync();
}

/**
 * @brief 根据备份点名称，删除备份点uuid记录
 * @param backupName 备份点名称
 */
void Utils::deleteBackupUniqueRecord(const QString& backupName)
{
    QString backupUniqueSetting = Utils::getSysRootPath() + UDISK_UNIQUE_SETTINGS;
    backupUniqueSetting.replace("//", "/");
    QSettings udisk_unique_settings(backupUniqueSetting, QSettings::IniFormat);
    udisk_unique_settings.setIniCodec(QTextCodec::codecForName("utf-8"));
    udisk_unique_settings.remove(backupName);
}

/**
 * @brief 获取备份点Uuid-BackupName键值对
 * @return Uuid-BackupName键值对
 */
QMap<QString, QString> Utils::getBackupUuidNameMap()
{
    // 1、udisk_unique_file文件中的信息
    QString backupUniqueSetting = Utils::getSysRootPath() + UDISK_UNIQUE_SETTINGS;
    backupUniqueSetting.replace("//", "/");
    QSettings udisk_unique_settings(backupUniqueSetting, QSettings::IniFormat);
    QStringList groups = udisk_unique_settings.childGroups();

    QMap<QString, QString> result;
    for (const QString& group : groups) {
        udisk_unique_settings.beginGroup(group);
        QString uuid = udisk_unique_settings.value("uuid").toString();
        udisk_unique_settings.endGroup();
        result.insert(uuid, group);
    }

    // 2、本地xml文件中的信息
    QString xmlPath = Utils::getSysRootPath() + BACKUP_XML_PATH;
    xmlPath.replace("//", "/");
    ParseBackupList parse(xmlPath);
    parse.getXmlUuidNameMap(result);

    return result;
}

/**
 * @brief 前后两次调用，然后取文件修改时间用于判断缓存数据是否落盘
 * @return
 */
bool Utils::updateSyncFile()
{
    QString fileIfSync = Utils::getSysRootPath() + FILE_IF_SYNC;
    fileIfSync.replace("//", "/");
    QFile file(fileIfSync);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << "sync" << END_LINE;
    out.flush();
    file.flush();
    file.close();

    return true;
}

/**
 * @brief 用事件循环替换sleep，以便事件尽量得到处理
 * @param sec，等待时间，单位秒
 * @author zhaominyong
 * @since 2021/05/29
 */
void Utils::wait(uint sec)
{
    QEventLoop loop;
    QTimer::singleShot(1000 * sec, &loop, SLOT(quit()));
    loop.exec();
}

/**
 * @brief 判断某目录是否某些目录的子目录
 * @param paths
 * @param path
 * @return
 */
bool Utils::isSubPath(const QStringList & paths, const QString& path)
{
    for (QString item : paths) {
        if (item.isEmpty() || item == Utils::getSysRootPath())
            continue;

        if (!item.endsWith("/"))
            item.append("/");

        if (path.startsWith(item))
            return true;
    }

    return false;
}

/**
 * @brief 获取备份还原版本号
 * @return
 */
QString Utils::getBackupVersion()
{
    QString version;
    Utils::executeCMD("dpkg -l yhkylin-backup-tools | grep yhkylin-backup-tools", version);
    // "ii  yhkylin-backup-tools 4.0.13-kylin72 amd64        YHkylin backup tools\n"
    QStringList fields = version.split(QRegularExpression("[ \t]+"));
    if (fields.size() >= 3)
        version = fields.at(2);
    else
        version = "none";

    return version;
}


/**
 * @brief 重新rw读写挂载efi分区
 */
void Utils::remountEfi()
{
    QString mountPath = Utils::getSysRootPath() + "/boot/efi";
    mountPath.replace("//", "/");
    QStringList args;
    args << "-o"
         << "rw,remount"
         << mountPath;
    Utils::executeCmd("mount", args);
}

/**
 * @brief 重新rw读写挂载boot分区
 */
void Utils::remountBoot()
{
    QString mountPath = Utils::getSysRootPath() + "/boot";
    mountPath.replace("//", "/");
    QStringList args;
    args << "-o"
         << "rw,remount"
         << mountPath;
    Utils::executeCmd("mount", args);
}


/**
 * @brief 停掉安全初始化服务
 */
void Utils::stopKysecInit()
{
    Utils::executeCmd("systemctl stop kysec-init.service");
}

/**
 * @brief 停掉网络服务
 */
void Utils::stopNetwork()
{
    Utils::executeCmd("service network-manager stop");
}

/**
 * @brief 文件保险箱清理
 */
void Utils::clearBox()
{
    // 创建一个DBus接口
        QDBusInterface backupInterface("com.kylin.boxmanage", "/backup", "com.kylin.boxmanage.backup", QDBusConnection::systemBus());

        // 调用DBus方法
            QDBusPendingReply<int> reply = backupInterface.call("format_allBoxRootEntries");

        // 检查调用是否成功
        int resultCode = reply.value();
        if (resultCode == 0) {
            // 操作成功
            qDebug() << "DBus method call succeeded."<< resultCode;;
        } else {
            // 操作失败
            qDebug() << "DBus method call failed with error code:" << resultCode;
            // 在这里处理失败的情况，比如输出报错信息
            // 注意：这里可以根据需要添加更详细的错误处理逻辑
            }
       
}



bool Utils::inhibit()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                             "/org/freedesktop/login1",
                                             "org.freedesktop.login1.Manager",
                                             QStringLiteral("Inhibit"));
    QString why("The backup and restore tool is busy, please retry after closed.");
    // 区分中英文
    QString locale = QLocale::system().name();
    if (locale == "zh_CN") {
        why = QString("备份还原工具正在运行，请关闭后再试");
    } else if (locale == "bo_CN") {
        why = QString("གྲབས་ཉར་སོར་ཆུད་ལག་ཆ་བྲེལ་བཞིན་ཡོད་།ཏོག་ཙམ་སྒུག་རོགས་།");
    }

    // QVariantList({what, who, why, mode})
    QVariantList args;
    args << QString("shutdown:sleep");
    args << QString("backup-daemon");
    args << why;
    args << QString("block");
    message.setArguments(args);

    QDBusPendingReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(message);

    if (!reply.isValid()) {
        qDebug() << "inhibit faile!";
        return false;
    }
    QDBusUnixFileDescriptor inhibitFileDescriptor = reply.value();
    inhibitFileDescriptor.swap(m_inhibitFileDescriptor);
    qDebug() << "inhibit success!";
    return true;
}

void Utils::uninhibit()
{
    if (!m_inhibitFileDescriptor.isValid()) {
        return;
    }

    qDebug() << "uninhibit success!";
    m_inhibitFileDescriptor = QDBusUnixFileDescriptor();
}
