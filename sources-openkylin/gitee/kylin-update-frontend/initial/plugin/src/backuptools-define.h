#ifndef BACKUPTOOLSDEFINE_H
#define BACKUPTOOLSDEFINE_H

namespace backuptools {

enum class backup_type {

    BACKUP_SYSTEM = 0, //写死，与auto-backup脚本中一样
    INC_BACKUP_SYSTEM = 1, //写死
    BACKUP_DATA = 2, //写死
    INC_BACKUP_DATA = 3, //写死

    RESTORE_SYSTEM = 4, //写死，与auto-backup脚本中一样

    RESTORE_SYSTEM_WITH_DATA,
    RESTORE_DATA,
    INC_RESTORE_DATA,

    DELETE_BACKUP,
    GHOST_IMAGE,

    BACKUP_TYPE_INIT,
};

enum class backup_state {
    FULL_BACKUP_SYS = 1, /* 正在进行全盘系统备份 */
    FULL_BACKUP_SYSUPDATE, /* 正在进行系统升级全盘备份 */
    FULL_BACKUP_DATA, /* 正在进行全盘数据备份 */
    INC_BACKUP_SYS, /* 正在进行增量系统备份 */
    INC_BACKUP_SYSUPDATE, /* 正在进行系统升级增量备份 */
    INC_BACKUP_DATA, /* 正在进行增量数据备份 */
    GHOST_BACKUP, /* 正在进行GHOST备份 */
    RESTORE_SYS, /* 正在进行系统还原 */
    INC_RESTORE_SYS, /* 正在进行增量系统还原 */
    RESTORE_DATA, /* 正在进行数据还原 */
    DELETE_POINT, /* 正在删除还原点 */
    CANCEL_BACKUP, /* 正在取消备份 */

    DU_ING,
    DU_FINISHED,
    UDISK_MKSQUASHFS_ING,
    UDISK_RSYNC_ING,

    BACKUP_STATE_INIT = 99, /* 没有正在备份或者还原任务 */
    // 备份准备中
    BACKUP_PREPARING,
    // 还原准备中
    RESTORE_PREPARING
};

enum class backup_result {

    BACKUP_RESULT_INIT,

    ETC_BOOTINFO_READ_FAIL,

    FSTAB_IS_NOT_EXIST,

    LOCK_PROGRAM_FAIL,
    OTHER_BACKUP_OR_RESTORE_RUNNING, //有其他正在进行的备份或者还原
    RM_RUNNING, //有删除任务正在进行
    MKSQUASHFS_RUNNING, //正在制作镜像

    NO_BLKID_EXIST,

    BACKUP_PARTITION_MOUNT_FAIL, //挂载/backup失败
    GENERATE_BACKUP_IMPORT_FILE_FAIL,

    BACKUP_CURRENT_DIR_IS_NOT_EXIST,

    BACKUP_SNAPSHOTS_DIR_IS_NOT_EXIST,

    BACKUP_STORAGEINFO_FILE_FAIL_TO_OPEN,

    BACKUP_STORAGEINFO_IS_NOT_CORRECT,

    WRITE_STORAGEINFO_ADD_ITEM_FAIL, //写入 /backup/snapshots/backuplist.xml 失败

    WRITE_STORAGEINFO_UPDATE_ITEM_FAIL,

    INC_NOT_FOUND_UUID, //无法找到 uuid

    INC_NOT_FOUND_DIR,

    WRITE_BACKUP_PATHS_TO_USER_FAILED,

    WRITE_EXCLUDE_BACKUP_PATHS_TO_USER_FAILED,

    BACKUP_CAPACITY_IS_NOT_ENOUGH, // /backup备份空间不足

    UDISK_FILESYSTEM_IS_READONLY,
    UDISK_STH_ERROR,
    UDISK_FILESYSTEM_TYPE_IS_VFAT,

    DU_ERR,

    BACKUP_CAPACITY_FOR_UDISKIMG_IS_NOT_ENOUGH,
    UDISK_CAPACITY_IS_NOT_ENOUGH,

    EFI_RSYNC_FAIL,

    BACKUP_PROCESS_START_FAIL, //备份启动失败
    BACKUP_PROCESS_DO_FAIL,

    BACKUP_START_SUCCESS,
    BACKUP_DO_SUCCESS,

    RESTORE_RESULT_INIT,

    RESTOREDIR_PREPARE_FAILED,

    RESTORE_PROCESS_START_FAIL,
    RESTORE_PROCESS_DO_FAIL,

    RESTORE_START_SUCCESS,
    RESTORE_DO_SUCCESS,

    GHOST_CANNOT_FIND_BACKUPPOINT,
    GHOST_SRC_DIRECTORY_IS_NOT_EXIST,
    GHOST_PROCESS_START_FAIL,
    GHOST_START_SUCCESS,
};

enum class delete_result {
    CANNOT_FIND_BACKUPPOINT,

    DIRECTORY_IS_NOT_EXIST,

    DELETE_BACKUPPOINT_FAIL,

    DELETE_BACKUPDIR_FAIL,
    START_DELETE_FAIL,

    DELETE_SUCCESS,
    START_DELETE_SUCCESS,
};

/*
* 系统更新自动备份
* 返回值：执行状态
* 0   : success.
* -1  : 写入 /backup/snapshots/backuplist.xml 失败.
* -2  : 备份空间不足.
* -3  : 备份启动失败.
* -4  : 备份分区不存在.
* -5  : 有其他正在进行的备份或者还原.
* -6  : 有删除任务正在进行.
* -7  : 正在制作镜像.
* -8  : 无法找到 uuid.
* -99: 打开/etc/.metainfo失败，或者为空
* -100: 挂载/backup失败
* -101: /backup/snapshots/backuplist.xml 不存在.
* -102: 打开 /backup/snapshots/backuplist.xml 失败.
* -103: /backup/snapshots/backuplist.xml 不正确.
* -104: 更新/backup/snapshots/backuplist.xml 失败.
* -105: 更新METAINFO 失败.
*/
#if 0
enum autobackup_result {
    AUTOBACKUP_START_SUCCESS,
    AUTOBACKUP_OTHER_BACKUP_OR_RESTORE_RUNNING,
    AUTOBACKUP_RM_RUNNING,
    AUTOBACKUP_MKSQUASHFS_RUNNING,
    AUTOBACKUP_MOUNT_BACKUP_FAIL,
    AUTOBACKUP_START_FAIL,

    AUTOBACKUP_RESULT_INIT,
};
#endif
}

#endif // BACKUPTOOLSDEFINE_H
