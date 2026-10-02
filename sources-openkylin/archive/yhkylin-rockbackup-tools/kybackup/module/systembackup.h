#ifndef SYSTEMBACKUP_H
#define SYSTEMBACKUP_H

#include <QStackedWidget>
#include "udiskdetector.h"
#include "../backup_manager_interface.h"
#include "../backup-daemon/parsebackuplist.h"
#include "../../common/dynamiccreator.h"

class SystemBackup : public QStackedWidget
{
    Q_OBJECT
public:
    enum SystemBackupState
    {
        IDEL = 0,  // 空闲
        CHECKING,  // 环境校验中
        BACKUPING  // 备份中
    };

    enum SystemBackupPage
    {
        HOME_PAGE,        // 首页
        SELECT_PATH_PAGE, // 选择备份路径页
        CHECK_ENV_PAGE,   // 环境检测页
        NAME_BACKUP_PAGE, // 备份命名页
        BACKUP_PAGE,      // 备份中页
        LAST_PAGE,        // 结束页
    };

    DECLARE_DYNCREATE_WITH_ONE_PARAM(SystemBackup, QStackedWidget*, QWidget*)

public:
    explicit SystemBackup(QWidget *parent = nullptr);
    ~SystemBackup();

private:
    void initHomePage();
    void initSetBackupPathsPage();
    void initCheckPage();
    void initNameBackupPage();
    void initBackupingPage();
    void initLastPage();

    QList<ParseBackupList::BackupPoint> getBackupPointList();
    bool isExistsBackupName(const QString & backupName);

signals:
    void startCheckEnv();
    void checkEnvResult(bool result, const QString &errMsg = "", const QString &errTip = "");
    void backupWarnning(const QString &warnning);
    void checkBackupResult(bool result, const QString &errMsg = "", const QString &errTip = "");
    void startBackup();
    void progress(int state, int rate);
    void clearBackupName();

public slots:
    void on_pre_clicked(bool checked = false);
    void on_next_clicked(bool checked = false);
    void on_checkEnv_start();
    void on_checkEnv_end(int result);
    void on_backup_start();
    void on_checkBackup_end(int result);
    void on_backup_end(bool result);

private:
    // U盘探测
    UdiskDetector* m_udector;
    // U盘挂载路径列表
    QStringList m_udiskPaths;
    // U盘列表变化时，先锁定
    bool m_isLock;
    // 是否本地备份
    bool m_isLocal;
    // 系统备份状态
    int m_systemBackupState;
    // 自定义路径
    QString m_customizePath;
    // 选中的备份目标路径前缀（暂指udisk挂载路径）
    QString m_prefixDestPath;
    // dbus接口
    ComKylinBackupManagerInterface *m_pInterface;
    // 备份点名称
    QString m_backupName;
};

#endif // SYSTEMBACKUP_H
