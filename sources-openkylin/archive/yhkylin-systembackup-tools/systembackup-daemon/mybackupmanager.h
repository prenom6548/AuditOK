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
    Q_CLASSINFO("D-Bus Interface", "com.kylin.systembackup.manager")

public:
    explicit MyBackupManager();
    virtual ~MyBackupManager();

//    QString getCallerInfo();
    QString getProcessName(uint pid);

signals:
    // 这3个信号供控制面板/自动更新等外围模块使用
    // 开始备份结果信号
    void sendStartBackupResult(int result);
    // 备份结果信号
    void sendAutoBackupResult(bool result);
    // 进度信号
    void sendRate(int, int);
    // 还原结果信号
    void sendAutoRestoreResult(bool result);


    //Kare相关信号
    // 备份Kare结果信号
    void sendAutoBackupKareResult(bool result);
    void sendAutoRestoreKareResult(bool result);


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
    // 开始ostree接口相关操作
    void startOstreeInterfaceOperation();

public slots:
    // 备份分区挂载
    int Mount_backup_partition();
    // 卸载backup分区
    bool umountBackupPartition();
    // 试用模式重新挂载分区
    bool remountPartitionsForCasperMode();
    // 环境检测
    int checkEnv(const BackupWrapper& backupWrapper);
    // 删除备份
    int deleteBackupPoint(const BackupWrapper& backupWrapper);
    // ghost镜像
    int ghostBackup(const BackupWrapper& backupWrapper);

    //系统升级备份接口
    void autoBackUpForSystemUpdate(const QString& backupName, const QString& frontUserName, int frontUid);
    // 控制面板调用的获取系统备份列表接口
    QString autoBackUpList();
    // 控制面板调用的系统自动备份点还原接口
    void autoRestoreForSystemUpdate(const QString& backupUuid,const QString& frontUserName, int frontUid);


    //Kare环境检测接口，返回0是成功，返回1是失败
    int checkEnvForKareUpdate(QString paths);
    //Kare备份还原接口
    void autoBackUpForKareUpdate(const QString& backupName, const QString& paths);
    bool checkKareBackupPoint();
    void autoRestoreForKareUpdate();

    //用户手动系统备份接口
    void backupSystemByUser(BackupWrapper backupWrapper);
    QString systemBackUpList();
    void restoreSystemByUser(BackupWrapper backupWrapper);

    //用户数据备份接口
    void backupDataByUser(BackupWrapper backupWrapper);
    void restoreDataByUser(BackupWrapper backupWrapper);

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
    // 唤醒后台daemon进程
    int wakeUpDaemon(const BackupWrapper& backupWrapper);

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

    //占屏
    void plymountMode();
    void plymountMessage(const QString& message);

    // 等待cancelWork和startOstreeInterfaceOperation两个信号都触发后执行的后续操作
    void doAfterBothSignals();



    /**
     * @brief 检测应用程序的环境变量是否有黑名单内的
     * @param pid，进程id
     * @return false-检测未通过，有安全风险；true-检测通过
     */
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

    //是否第一次发送100%信号
    bool m_firstTime = true;

    // 是否开始执行ostree接口相关操作
    bool m_isStartOstreeInterfaceOperation = false;

};

#endif // MYBACKUPMANAGER_H
