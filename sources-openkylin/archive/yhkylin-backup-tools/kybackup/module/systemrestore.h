#ifndef SYSTEMRESTORE_H
#define SYSTEMRESTORE_H

#include <QStackedWidget>
#include "../backup_manager_interface.h"
#include "../backup-daemon/parsebackuplist.h"
#include "../../common/dynamiccreator.h"

class SystemRestore : public QStackedWidget
{
    Q_OBJECT
public:
    enum SystemRestoreState
    {
        IDEL = 0,  // 空闲
        CHECKING,  // 环境校验中
        RESTORING  // 还原中
    };

    enum SystemRestorePage
    {
        HOME_PAGE,        // 首页
        CHECK_ENV_PAGE,   // 环境检测页
        RESTORE_PAGE,      // 还原中页
        LAST_PAGE,        // 结束页
    };

    DECLARE_DYNCREATE_WITH_ONE_PARAM(SystemRestore, QStackedWidget*, QWidget*)

public:
    explicit SystemRestore(QWidget *parent = nullptr);
    virtual ~SystemRestore();

private:
    void initHomePage();
    void initCheckPage();
    void initRestoringPage();
    void initLastPage();

signals:
    void startCheckEnv();
    void checkEnvResult(bool result, const QString &errMsg = "", const QString &errTip = "");
    void startRestore();
    void progress(int state, int rate);
    void checkRestoreResult(bool result, const QString &errMsg = "", const QString &errTip = "");

public slots:
    void on_pre_clicked(bool checked = false);
    void on_next_clicked(bool checked = false);
    void on_button_beginRestore_clicked(bool checked = false);
    void on_checkEnv_start();
    void on_checkEnv_end(int result);
    void on_restore_start();
    void on_checkRestore_end(int result);
    void on_restore_end(bool result);

private:
    // 是否保留用户数据
    bool m_isRetainUserData;
    // 是否出厂还原
    bool m_isFactoryRestore;

    // dbus接口
    ComKylinBackupManagerInterface *m_pInterface;

    QString m_uuid; // 还原点的UUID
    QString m_devPath; // 如果是从移动设备进行还原，此中保存移动设备挂载路径
    QString m_backupName; // 还原点的备份点名称
    bool m_isOtherMachine; // 是否异机备份点还原
    // 本地备份还是U盘备份: 0-本地备份；1-移动设备备份；2-异机备份（仅用于业务场景标记，不用于持久化记录）；3-自定义路径备份
    int m_iPosition;
    // 系统备份状态
    int m_systemRestoreState;
};

#endif // SYSTEMRESTORE_H
