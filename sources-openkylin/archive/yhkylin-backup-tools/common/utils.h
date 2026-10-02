#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QtDebug>
#include "mydefine.h"

#include <QDBusUnixFileDescriptor>

class SystemInfo
{
public:
    // 操作系统
    static QString m_os;
    // 架构
    static QString m_arch;
    // 引导方式
    static QString m_archDetect;
};

class PartitionInfo
{
public:
    QString m_device;
    QString m_label;
    QString m_uuid;
    QString m_type;
};

/**
 * @brief 工具类
 * @author zhaominyong
 * @since 2021/07/22
 */
class Utils
{
public:
    /**
     * @brief initSysRootPath, 根据应用程序路径推断系统根目录
     * @note
     *      本方法依赖于根分区挂载目录
     *      1. grub引导中根目录为/root（现在grub菜单的系统备份还原改为了initrd阶段用shell脚本实现，不会再用此应用程序）
     *      2. 试安装的小系统中根目录约定为/target（试安装小系统中进行系统还原的功能暂未提供）
     *      3. 正常使用中根目录为/
     *      此接口是为试安装的小系统中进行系统还原预留
     */
    static void initSysRootPath();

    /**
     * @brief getSysRootPath，获取正式系统根目录
     * @return const QString&，系统根目录
     * @note
     *      调用此函数前，需要调用初始化系统根目录函数initSysRootPath；否则，返回默认目录“/”
     */
    static const QString& getSysRootPath() { return m_sysRootPath; }

    static const QString& getBackupRootPath() { return m_backupRootPath; }

    /**
     * @brief /proc/cmdline中包含boot=casper则说明是运行的试安装的小系统
     * @return true-是试安装模式中；false-不是
     */
    static bool isCasper();

    /**
     * @brief customMessageHandler，日志重定向句柄
     * @param type 日志类型，debug等
     * @param context 上下文
     * @param msg 日志信息
     */
    static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

    /**
     * @brief 文件锁，锁定应用程序
     * @param frontUid，锁定程序的用户id
     * @return 锁文件的句柄
     */
    static int lockProgram(int frontUid);

    /**
     * @brief 解除应用程序文件锁
     * @param lock_file_fd 锁文件的文件描述符
     * @return 0，解除成功；1，解除失败
     */
    static int unLockProgram(int lock_file_fd);

    /**
     * @brief 删除锁文件
     * @return bool
     */
    static bool rmLockFile();

    /**
     * @brief 检查/etc/.bootinfo是否存在并可读，里面存放备份分区的UUID等信息
     * @return bool
     */
    static bool checkBootInfoExists();

    /**
     * @brief 是否有平板模式
     * @return bool
     */
    static bool isTablet();

    /**
     * @brief 获取备份分区的UUID
     * @return 备份分区的UUID
     */
    static QString getBackupPartitionUuid();

    /**
     * @brief 获取分区对应的UUID映射关系
     * @param fstab文件路径
     * @return “分区挂载目录-分区UUID”键值对
     */
    static QHash<QString, QString> getPartUuidMap(const QString &fstab);

    /**
     * @brief swapKeyValue
     * @param hash
     * @return
     */
    static QHash<QString, QString> swapKeyValue(const QHash<QString, QString>& hash);

    /**
     * @brief /etc/fstab中的绑定映射关系
     * @return src-dest键值对map
     */
    static QHash<QString, QString> getFstabBindInfo();

    /**
     * @brief 路径不存在则创建，不支持递归创建
     * @param path
     */
    static void mkdir(const QString& path);

    /**
     * @brief 创建路径，支持递归创建
     * @param path
     * @return bool
     */
    static bool mkpath(const QString& path);

    /**
     * @brief 备份还原时，需要排除绑定挂载的路径，因为数据不是存放在挂载路径内
     * @param excludes，存放需要排除的绑定路径
     */
    static void excludeFstabBindPath(QStringList &excludes);

    /**
     * @brief 备份还原时，需要一些用户家目录下的路径，因为这些路径备份还原程序无权操作
     * @param excludes，存放需要排除的路径
     */
    static void excludeSomeHomePath(QStringList &excludes);

    /**
     * @brief 获取指定目录的子目录列表
     * @param parent，指定的父目录
     */
    static QStringList getSubDirs(const QString& parent);

    /**
     * @brief 获取额外备份或还原数据
     * @param extraPaths，存放额外需要备份或还原的数据路径列表
     * @note 主要应用于系统更新备份/还原、保留用户数据还原，需要备份\还原这些额外数据；否则，可能引起某些软件的用户配置和软件版本不匹配造成的不能正常使用
     */
    static void extraBackupOrRestorePaths(QStringList &extraPaths);

    /**
     * @brief 排除自定义备份路径
     * @param excludes，存放需要排除的路径
     */
    static void excludeCustomizePath(QStringList &excludes);

    /**
     * @brief 生成rsync --exclude-from排除路径规则文件
     * @return
     */
    static bool generateExcludePathsFile();

    /**
     * @brief 获取系统备份还原排除路径列表
     * @return 系统备份还原排除路径列表
     */
    static QStringList getFromExcludePathsFile();

    /**
     * @brief 判断目录是否存在
     * @param 目录路径
     * @return true,存在；false,不存在
     */
    static bool isDirExist(const QString& fullDirName);

    /**
     * @brief 生成Uuid
     * @return UUID
     */
    static QString createUuid();

    /**
     * @brief 将列表中内容写入指定文件中
     * @param fileName，文件全路径
     * @param lines，内容列表
     * @return true，成功写入；false，写入失败
     */
    static bool writeFileByLines(const QString& fileName, const QStringList& lines);

    /**
     * @brief 判断文件是否存在
     * @param fileName 文件明
     * @return bool，true-存在；false-不存在
     * @author zhaominyong
     * @since 2021/05/29
     */
    static bool fileExists(const QString &fileName);

    /**
     * @brief 判断目录是否为空
     * @param fullDirName 目录路径
     * @return true-目录不存在或为空目录; false-非空目录
     */
    static bool isDirEmpty(const QString& fullDirName);

    /**
     * @brief 记录备份日志
     * @param line，日志内容
     * @return bool, true-成功；false-失败
     * @author zhaominyong
     * @since 2021/05/29
     * @note
     *      因为系统恢复成功后马上需要reboot，但是此时的文件缓存可能还未能落盘，故此增加此函数，其中调用了系统函数fdatasync确保缓存落盘
     */
    static bool writeBackupLog(QString line);

    /**
     * @brief 立即写文件
     * @param fileName 文件名，包含路径
     * @param content 文件内容
     * @return bool
     */
    static bool syncWriteFile(const QString &fileName, const QString& content);

    /**
     * @brief 将字节大小转换为GB等表示的字符串
     * @param size，qint64，空间大小，单位字节
     * @return GB/MB/KB等表示的字符串型大小
     */
    static QString StringBySize(qint64 size);

    /**
     * @brief 获取挂接的移动设备列表
     * @return MOUNTPOINT,PATH键值对列表(path中如果含有空格时显示不对，建议不用此值)
     * @author zhaominyong
     * @since 2021/06/07
     * @note
     *      for bug  59636 【cpm】【HUAWEI】【KOS】【L0】【V3试制】备份时选择移动设备，会出现一个dev/sdc的路径(一般+必现+不常用功能)
     *      QStorageInfo::mountedVolumes获取的磁盘列表区分不出来是否移动设备
     */
    static QHash<QString, QString> getRemovableStorages();

    /**
     * @brief 获取挂接的计算机内部磁盘
     * @return 内部磁盘挂接路径列表
     */
    static QList<QString> getLocalDisks();

    /**
     * @brief 获取分区信息
     * @return “LABEL-分区信息”键值对
     */
    static QHash<QString, PartitionInfo> getPartitions();

    /**
     * @brief 向文件里面覆盖写文本内容
     * @param fileName
     * @param text
     * @return true-成功; false-失败
     */
    static bool writeFile(const QString& fileName, const QString& text);

    static void echoKysecStatus(bool flag);

    /**
     * @brief 设置安全状态
     * @param enable——true，开启保护；false，关闭保护
     */
    static void setKysecStatus(bool enable);

    /**
     * @brief 启动或关闭kysec-sync-daemon服务
     * @param enable——true，开启；false，关闭
     */
    static void setKysecDaemon(bool enable);

    /**
     * @brief 判断程序是否已开启
     * @param processName
     * @return true-正在运行；false-未运行
     */
    static bool isRunning(const QString &processName);

    /**
     * @brief 判断是否990或9006C处理器
     * @return bool
     */
    static bool isHuawei990();

    /**
     * @brief popen()函数通过创建一个管道，调用fork()产生一个子进程，执行一个shell以运行命令来开启一个进程。
     * @param cmd，是一个指向以 NULL 结束的 shell 命令字符串的指针。这行命令将被传到 bin/sh 并使用 -c 标志，shell 将执行这个命令
     * @param result 输出结果
     * @note 不建议桌面应用调用，因为会依赖终端使得开机自启无法启动起来
     */
    static void executeCMD(const char* cmd, QString &result);
    static QString executeCmd(const QString &cmd, const QStringList &args = QStringList());

    /**
     * @brief 在终端中执行简单命令
     * @param cmd
     * @return 返回终端输出结果
     */
    static QString processCmd(const QString& cmd);

    /**
     * @brief 获取备份日志列表
     * @return
     */
    static QList<BackupWrapper> getBackupLogList();

    /**
     * @brief 初始化系统信息
     */
    static void initSystemInfo();

    /**
     * @brief getOs
     * @return 操作系统名字, 如：
     *         Kylin-Desktop V10-SP1
     *         Build 20210407
     */
    static QString getOs();

    /**
     * @brief getArch
     * @return 架构，arch命令的结果，如：x86_64
     */
    static QString getArch();

    /**
     * @brief getArchDetect
     * @return archdetect命令的结果，如：amd64/efi
     */
    static QString getArchDetect();

    /**
     * @brief 获取分区剩余大小
     * @return
     */
    static QMap<QString, QString> getLeftSizeOfPartitions();

    /**
     * @brief 获取分区可用大小，主要是/和/data分区
     * @return 路径和大小（单位：字节）键值对
     */
    static QHash<QString, qint64> getAvailableSizeOfPartitions();

    /**
     * @brief 获取文件夹或文件的大小
     * @param path 路径
     * @return 大小
     */
    static qint64 getDirOrFileSize(const QString &path);

    /**
     * @brief 记录下备份点uuid及其名称
     * @param uuid
     * @param backupName
     */
    static void update_backup_unique_settings(const QString &uuid, const QString &backupName);

    /**
     * @brief 根据备份点名称，删除备份点uuid记录
     * @param backupName 备份点名称
     */
    static void deleteBackupUniqueRecord(const QString& backupName);

    /**
     * @brief 获取备份点Uuid-BackupName键值对
     * @return Uuid-BackupName键值对
     */
    static QMap<QString, QString> getBackupUuidNameMap();

    /**
     * @brief 前后两次调用，然后取文件修改时间用于判断缓存数据是否落盘
     * @return
     */
    static bool updateSyncFile();

    /**
     * @brief 用事件循环替换sleep，以便事件尽量得到处理
     * @param sec，等待时间，单位秒
     * @author zhaominyong
     * @since 2021/05/29
     */
    static void wait(uint sec);

    /**
     * @brief 判断某目录是否某些目录的子目录
     * @param paths
     * @param path
     * @return
     */
    static bool isSubPath(const QStringList & paths, const QString& path);

    /**
     * @brief 获取备份还原版本号
     * @return
     */
    static QString getBackupVersion();

    /**
     * @brief 重新rw读写挂载efi分区
     */
    static void remountEfi();

    /**
     * @brief 重新rw读写挂载boot分区
     */
    static void remountBoot();

    /**
     * @brief 停掉安全初始化服务
     */
    static void stopKysecInit();

    /**
     * @brief 停掉网络服务
     */
    static void stopNetwork();

    /**
     * @brief 设置日志文件
     * @param logfile
     */
    static void setLogFile(const QString& logfile) { m_logPath = logfile; }

    /**
     * @brief 文件保险箱清理
     */
    static void clearBox();


    /**
     * @brief 阻止session关机用dbus新接口Inhibit来替换锁文件的方式
     * Inhibti函数有四个参数，按顺序分别为：
            what:需要阻止的是哪些过程，如关机、重启；可以输入的值有：”shutdown”(会同时阻止关机重启)，”sleep”（会同时阻止睡眠休眠），需要同时阻止关机和睡眠可以输入 ”shutdown:sleep”
            who:是谁要阻止，调用该接口的程序名，如”ukui-sessions”
            why：阻止的原因，简单描述即可，如”package installing”
            mode:阻止的方式；可输入的值有”block”(无限期阻止，等到主动取消阻止才会取消)，”delay”（超时阻止，超时一段时间后不管阻止锁，直接执行）
            建议使用”block”的方式阻止，加锁后记得解锁
            可以使用同服务下ListInhibitors方法获取当前的inhibit锁，测试验证是否成功注册
     * @return
     */
    static bool inhibit();
    static void uninhibit();

private:
    // 系统根目录，默认"/"
    static QString m_sysRootPath;
    static QString m_backupRootPath;
    // 日志路径，不设置则默认/var/log/backup.log
    static QString m_logPath;

    //用于inihibit控制
    static QDBusUnixFileDescriptor m_inhibitFileDescriptor;
};

#endif // UTILS_H
