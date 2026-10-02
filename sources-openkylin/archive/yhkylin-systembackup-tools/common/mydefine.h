#ifndef MYDEFINE_H
#define MYDEFINE_H

#include <QString>
#include <QStringList>
#include <QMetaType>
#include <QDBusArgument>

#define DEFAULT_APP_PATH "/usr/bin"

#define BACKUP_PATH "/backup"
#define BACKUP_SNAPSHOTS_PATH "/.backup/snapshots"
#define BACKUP_XML_PATH "/.backup/snapshots/backuplist.xml"
#define UDISK_BACKUP_SNAPSHOTS_PATH "/backup/snapshots"
#define UDISK_BACKUP_XML_PATH "/backup/snapshots/udisk_backuplist.xml"
#define SYS_KARE_BACKUP_XML_PATH "/.backup/snapshots/sys_kare_backuplist.xml"

#define SYS_UPDATE_BACKUP_XML_PATH "/.backup/snapshots/sys_update_backuplist.xml"
#define SYS_BACKUP_XML_PATH "/.backup/snapshots/sys_backuplist.xml"
#define FACTORY_BACKUP_XML_PATH "/.backup/snapshots/factory_backuplist.xml"
#define USER_XML "/snapshots/user_backuplist.xml"
#define SYSTEM_XML "/.backup/snapshots/sys_backuplist.xml"
#define UDISK_XML "/snapshots/sys_backuplist.xml"
#define EXCLUDE_FILE_PATH "/backup/snapshots/.exclude"
#define CHECK_PATH "/backup/snapshots/check/data/"
#define BACKUP_LOG_TEXT_PATH "/.backup/log.txt"
#define BACKUP_IMGBACKUP_PATH "/backup/imgbackup"
#define IMGBACKUP_PATH "/imgbackup"
#define UDISK_MKSQUASHFS_IMG_NAME "dst.img"
#define UDISK_UNIQUE_SETTINGS "/backup/udisk_unique_file"
#define GHOST_PATH "/ghost"

#define DATA_PATH "/data"
#define CASPER_ROOT_PATH "/target"
#define BOOTINFO_PATH "/etc/.bootinfo"
#define FSTAB_PATH "/etc/fstab"
#define LOCK_FILE "/tmp/lock/kylin-backup.lock"
#define LOCK_FILE_PATH "/tmp/lock"
#define PROC_LOG "/data/.backup/backup.log"

#define FILE_SAVE_SIZE "/.backup/file_save_size"

#define FILE_IF_SYNC "/.backup/file_if_sync"
#define EXTRA_BACKUP_OR_RESTORE_CONF "/etc/userdata_backup_or_restore.conf"
#define FLAG_FILE_SYSTEM_UPDATE_NEED_ROLLBACK "/var/lib/kylin-system-updater/need-rollback"

#define BACKUP_CLI_NAME "kybackup"

#define AUTO_BACKUP_UUID "{01234567-0123-0123-0123-0123456789ab}"
#define FACTORY_BACKUP_UUID "{00000000-0000-0000-0000-000000000000}"
#define CUSTOMIZEKEYBOARD_SYSTEM_BACK_UP_UUID "{11234567-0123-0123-0123-0123456789ab}"

#define PATHS_USER_FILE ".user.txt"
#define EXCLUDE_PATHS_USER_FILE ".exclude.user.txt"
#define EXTRA_PATHS_USER_FILE ".user_extra.txt"

#define BACKUP_PARSE_STATE_SUCCESS_STRTING "backup finished"
#define BACKUP_PARSE_STATE_FAIL_STRTING "backup unfinished"
#define BACKUP_PARSE_STATE_INC_SUCCESS_STRTING "inc backup finished"
#define BACKUP_PARSE_STATE_INC_FAIL_STRTING "inc backup unfinished"

#define THEME_YHKYLIN_BACKUP_TOOLS "yhkylin-backup-tools"

#define PID_STRING_LEN 1024
#define DEFAULT_MAX_SIZE 16777215

#define NEEDROOLBACKFILE "/data/.backup/roolback.txt"
#define NEEDUNDEPLOYFILE "/data/.backup/undeploy.txt"
#define NEEDROOLBACKKAREFILE "/data/.backup/kare-roolback"

// 蓝色
#define COLOR_BLUE "#3790FA"
// 蓝黑
#define COLOR_BLUE_DARK "#3B4251"
// 浅灰
#define COLOR_LIGHT_DARK "#F4F4F4"
// 灰色
#define COLOR_GRAY "#CCCCCC"
// 深灰
#define COLOR_DARK_GRAY "#393A3E"
// 黑灰
#define COLOR_BLACK_GRAY "#2D2E32"
// 字体灰色
#define COLOR_DISABLE_GRAY "#737373"
// 黄色
#define COLOR_YELLOW "#F8A34C"
// 浅蓝
#define COLOR_LIGHT_BLUE "#DDEBFF"

#define END_LINE "\n"

// 分区label
#define LABEL_SYSROOT "SYSROOT"
#define LABEL_EFI "ESP"
#define LABEL_SYSBOOT "SYSBOOT"
#define LABEL_KYLINBACKUP "KYLIN-BACKUP"
#define LABEL_DATA "DATA"

#define PC_TITLE_BAR_SIZE 30
#define TABLET_TITLE_BAR_SIZE 48

/**
 * @brief 备份还原操作类型
 */
enum BackupType {
    // 系统备份
    BACKUP_SYSTEM = 0,
    // 增量系统备份
    INC_BACKUP_SYSTEM = 1,
    // 数据备份
    BACKUP_DATA = 2,
    // 增量数据备份
    INC_BACKUP_DATA = 3,
    // 系统还原
    RESTORE_SYSTEM = 4,
    // 保留用户数据还原
    RESTORE_SYSTEM_WITH_DATA = 5,
    // 数据还原
    RESTORE_DATA = 6,
    // 增量数据还原，实际没有这个类型，为兼容以前的数据仍然保留
    INC_RESTORE_DATA = 7,
    // 删除备份点
    DELETE_BACKUP = 8,
    // ghost镜像
    GHOST_IMAGE = 9,
    // 自动系统备份，为了兼容以前版本及简化处理后面会转为BACKUP_SYSTEM（系统备份），此处仅为了方便进行任务分派
    AUTO_BACKUP_SYSTEM = 10,

    //系统升级还原
    AUTO_RESTORE_SYSTEM = 11,
    // 仅仅初始化BackupType枚举类变量用，没有用于持久化，不用固定其值
    BACKUP_TYPE_INIT,
};

/**
 * @brief 备份信息结构体
 */
struct BackupWrapper {
    // 操作类型，如：系统备份, 系统还原
    int m_type = -1;
    // 本地备份还是U盘备份: 0-本地备份；1-移动设备备份;2-异机备份；3-自定义路径
    int m_iPosition = -1;
    // 操作的用户类型: 0-自动操作;1-用户操作
    int m_User = -1;
    // 备份名称，用于识别备份的，默认是时间
    QString m_comment;
    // 备份名称，用来替换m_comment
    QString m_backupName;
    // 备份或还原指定的UUID
    QString m_uuid;
    // 待备份目录
    QStringList m_backupPaths;
    // 备份需要排除的路径
    QStringList m_backupExcludePaths;
    // 备份目标路径/还原点路径（前缀）: 1、移动设备挂载路径,在向移动设备备份（或从移动设备还原）时使用它来指定对应的移动设备路径 2、指定备份路径
    QString m_prefixDestPath;
    // 备注信息
    QString m_note;
    // 备份用户名
    QString m_frontUserName;
    // 备份用户id
    int m_frontUid = -1;
    // 备份用户所属组id
    int m_gid = -1;
    // 是否异机备份点: 0-本机备份；1-异机备份
    int m_isOtherMachine = 0;

    // 下面参数不是入参，是服务端自行跟随业务场景设置
    // 是否增量备份
    bool m_bIncrement = false;
    // 新增备份点时增量备份的基准uuid
    QString m_baseUuid;
    // 备份时间(暂用于日志查询)
    QString m_time;
    // 备份大小(暂用于日志查询)
    QString m_size;

    //新版本新增接口参数
    QString m_commitHash;

    //系统数据和用户数据分离，需要标记当前是哪种类型 0：用户数据  1: 系统数据
    int m_backupType = -1;

    static void registerMetaType();
};
Q_DECLARE_METATYPE(BackupWrapper)

extern QDBusArgument &operator<<(QDBusArgument &argument, const BackupWrapper &backupWrapper);

extern const QDBusArgument &operator>>(const QDBusArgument &argument, BackupWrapper &backupWrapper);

// 用户名
extern QString g_userName;

// 为兼容旧版本对外提供的接口，此处结构前面暂时不变，只在后面增加
enum class MountResult {
    MOUNT_RESULT_INIT,
    CANNOT_GET_BACKUPUUID,
    NO_BLKID_EXIST,
    NO_MOUNTED,
    GENERATE_IMPORT_FILE_FAIL,
    MOUNTED
};

/**
 * @brief 备份还原工具状态
 */
enum class BackupState {
    // 准备中
    PREPARING = 1,
    // 工作中
    WORKING,

    // 空闲状态
    BACKUP_STATE_INIT = 99
};

/**
 * @brief 备份位置
 */
enum BackupPosition
{
    LOCAL, // 本地磁盘
    UDISK, // 移动设备
    OTHER,  // 移动设备中的其它机器的备份点
    CUSTOMIZE, // 自定义路径
};

enum BackupUser
{
    AUTO, // 自动执行
    USER, // 用户手动执行
};

/**
 * @brief 备份还原进展
 * @note 为兼容以前老版本提供给自动更新模块的接口，此结构前面部分保持老代码结构
 */
enum class BackupResult {
    // 备份初始
    BACKUP_RESULT_INIT,
    // /etc/.bootinfo读取失败
    ETC_BOOTINFO_READ_FAIL,
    // /etc/fstab不存在
    FSTAB_IS_NOT_EXIST,
    // 文件锁定应用失败
    LOCK_PROGRAM_FAIL,
    //有其他正在进行的备份或者还原
    OTHER_BACKUP_OR_RESTORE_RUNNING,
    // 有删除任务正在进行
    RM_RUNNING,
    // 正在制作镜像
    MKSQUASHFS_RUNNING,
    // 设备uuid不存在
    NO_BLKID_EXIST,
    // 挂载/backup分区失败
    BACKUP_PARTITION_MOUNT_FAIL,
    // 生成备份导入文件失败
    GENERATE_BACKUP_IMPORT_FILE_FAIL,
    // /backup/current目录不存在
    BACKUP_CURRENT_DIR_IS_NOT_EXIST,
    // /backup/snapshots目录不存在
    BACKUP_SNAPSHOTS_DIR_IS_NOT_EXIST,
    // /backup/snapshots/backuplist.xml文件不存在或打开失败
    BACKUP_STORAGEINFO_FILE_FAIL_TO_OPEN,
    // /backup/snapshots/backuplist.xml格式不正确
    BACKUP_STORAGEINFO_IS_NOT_CORRECT,
    // 写入 /backup/snapshots/backuplist.xml 失败
    WRITE_STORAGEINFO_ADD_ITEM_FAIL,
    // 修改 /backup/snapshots/backuplist.xml 失败
    WRITE_STORAGEINFO_UPDATE_ITEM_FAIL,
    // 增量备份未找到对应的uuid
    INC_NOT_FOUND_UUID,
    // 增量备份(或还原)未找到对应的目录
    INC_NOT_FOUND_DIR,
    // 将备份路径写入(或读出)/backup/snapshots/{uuid}/.user.txt失败
    WRITE_BACKUP_PATHS_TO_USER_FAILED,
    // 将备份路径写入(或读出)/backup/snapshots/{uuid}/.exclude.user.txt失败
    WRITE_EXCLUDE_BACKUP_PATHS_TO_USER_FAILED,
    // /backup备份空间不足
    BACKUP_CAPACITY_IS_NOT_ENOUGH,
    // U盘文件系统是只读的
    UDISK_FILESYSTEM_IS_READONLY,
    // u盘中创建目录/backup/snapshots失败
    UDISK_STH_ERROR,
    // u盘文件系统是vfat格式（容量有限或单个文件大小有限）
    UDISK_FILESYSTEM_TYPE_IS_VFAT,
    // u盘文件系统是vfat格式（容量有限或单个文件大小有限）
    UDISK_FILESYSTEM_TYPE_IS_NOT_EXT4,
    // DU计算大小失败
    DU_ERR,
    // 向U盘备份时先压缩到本地成img文件，本地备份分区空间不足
    BACKUP_CAPACITY_FOR_UDISKIMG_IS_NOT_ENOUGH,
    // U盘容量不足
    UDISK_CAPACITY_IS_NOT_ENOUGH,
    // /boot/efi同步失败
    EFI_RSYNC_FAIL,
    // 备份启动失败
    BACKUP_PROCESS_START_FAIL,
    // 备份失败
    BACKUP_PROCESS_DO_FAIL,

    // 备份进程成功启动
    BACKUP_START_SUCCESS,
    // 备份成功
    BACKUP_DO_SUCCESS,

    // 还原初始
    RESTORE_RESULT_INIT,
    // 还原目录准备失败
    RESTOREDIR_PREPARE_FAILED,
    // 还原进程开启失败
    RESTORE_PROCESS_START_FAIL,
    // 还原失败
    RESTORE_PROCESS_DO_FAIL,
    // 还原成功开始
    RESTORE_START_SUCCESS,
    // 还原成功
    RESTORE_DO_SUCCESS,

    // ghost镜像未找到备份点
    GHOST_CANNOT_FIND_BACKUPPOINT,
    // ghost源目录不存在
    GHOST_SRC_DIRECTORY_IS_NOT_EXIST,
    // ghost进程启动失败
    GHOST_PROCESS_START_FAIL,
    // ghost进程启动成功
    GHOST_START_SUCCESS,

    // 后面的为新版本新增部分

    // 根据操作类型动态创建处理类失败
    NO_FOUND_DEALCLASS,
    // 环境检测成功
    CHECK_ENV_SUCCESS,
    // mksquashfs启动成功
    MKSQUASHFS_START_SUCCESS,
    // mksquashfs压缩img文件失败
    MKSQUASHFS_DO_FAIL,
    // mksquashfs压缩img文件成功，开始转移img文件到u盘
    MKSQUASHFS_DO_SUCCESS,
    // 开始取消操作
    START_CANCEL,
    // 取消操作成功
    CANCEL_SUCCESS,

    //结合ostree新增
    CHECK_REPO_SUCCESS,
    CHECK_REPO_FAIL,
    CHECK_COMMITHASH_FAIL,

    //出厂还原
    CHECK_SYSTEMHASH_FAIL,

    //判断U盘类型是否是vfat
    CHECK_UDISK_TYPE_FAIL,

    // u盘文件系统是udf格式
    UDISK_FILESYSTEM_TYPE_IS_UDF
};

#endif // MYDEFINE_H
