#ifndef MYBACKUPMANAGER_H
#define MYBACKUPMANAGER_H

#include <QObject>
#include <QDBusContext>
#include <QtDBus/QDBusVariant>
#include <QHash>
#include <QDBusUnixFileDescriptor>
#include "mythread.h"
#include "../common/mydefine.h"
#include "../common/spinlock_mutex.h"

class MyBackupManager : public QObject, protected QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.backup.manager")

public:
    explicit MyBackupManager();
    virtual ~MyBackupManager();

signals:
    // 这3个信号供控制面板/自动更新等外围模块使用
    // 开始备份结果信号
    void sendStartBackupResult(int result);
    // 备份结果信号
    void sendBackupResult(bool result);
    // 进度信号
    void sendRate(int, int);
    // 还原结果信号
    void sendAutoRestoreResult(bool result);

    // 模块内使用的信号
    // 进度信号
    void progress(int, int);
    // 备份结果信号
    void backupFinished(bool result);
    // 环境检查结果信号
    void sendEnvCheckResult(int result);
    // 还原结果信号
    void sendRestoreResult(bool result);
    // 删除备份结果信号
    void sendDeleteResult(bool result);
    // ghost备份结果信号
    void sendGhostBackupResult(bool result);

public slots:
    // 备份分区挂载
    int Mount_backup_partition();
    // 卸载backup分区
    bool umountBackupPartition();
    // 试用模式重新挂载分区
    bool remountPartitionsForCasperMode();
    // 环境检测
    int checkEnv(const BackupWrapper& backupWrapper);
    // 备份
    int goBackup(const BackupWrapper& backupWrapper);
    // 还原
    int goRestore(const BackupWrapper& backupWrapper);
    // 删除备份
    int deleteBackupPoint(const BackupWrapper& backupWrapper);
    // ghost镜像
    int ghostBackup(const BackupWrapper& backupWrapper);
    // 控制面板调用的备份接口，重构暂时先兼容以前的老接口
    void autoBackUpForSystemUpdate_noreturn(const QString& backupName, const QString& createNote, const QString& incNote, const QString& frontUserName, int frontUid);
    // 控制面板调用的系统自动备份点还原接口
    void autoRestoreForSystemUpdate(const QString& frontUserName, int frontUid);
    // 控制面板调用的获取系统备份接口
    QString getBackupCommentForSystemUpdate(QString& state);
    // 获取备份状态
    int getBackupState(bool& isActive);
    // 取消操作
    int cancel();
    // 重启操作
    int reboot();
    // 任务结束
    void finished();
    // 是否存在系统备份节点
    bool existsSystemBackupNode();
    // plymouth出厂还原场景时，翻译不生效;root启动的服务程序locale为"C",不是"zh_CN"之类的
    void setLocale(const QString& locale);

private slots:
    // 卸载backup分区
    bool umountBackup();

private:
    // 锁定
    bool lock(int frontUid);
    bool inhibit();
    // 解锁
    bool unlock();
    void uninhibit();



    /**
     * @brief 检测应用程序的环境变量是否有黑名单内的
     * @param pid，进程id
     * @return false-检测未通过，有安全风险；true-检测通过
     */
    // TUDO:dbus管控优化
    //static bool checkEnviron(int pid);

    /**
     * @brief 通过白名单判断调用放是否允许调用此dbus服务
     * @return true-允许；false-禁止
     */
    bool isAllowedCaller();

private:
    // 自旋锁
    spinlock_mutex m_mutex;
    // 锁文件描述符
    int m_fdLockFile = -1;
    QDBusUnixFileDescriptor m_inhibitFileDescriptor;
    // 备份还原状态
    BackupState m_backupState = BackupState::BACKUP_STATE_INIT;
    // 是否活动的，兼容以前用的,和m_backupState的实际使用功能有重复
    bool m_isActive = false;
    bool m_needUmount = false;
    bool m_bOpenKysec = false;
    bool m_bStartKysecDeamon = false;
    // 工作者线程
    MyThread workerThread;

    // 允许调用dbus接口的应用程序白名单
    QStringList m_programWhiteList;

};

#endif // MYBACKUPMANAGER_H
