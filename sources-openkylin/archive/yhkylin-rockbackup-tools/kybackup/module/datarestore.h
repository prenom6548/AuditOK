#ifndef DATARESTORE_H
#define DATARESTORE_H

#include <QStackedWidget>
#include "../backup_manager_interface.h"
#include "../backup-daemon/parsebackuplist.h"
#include "../../common/dynamiccreator.h"

class DataRestore : public QStackedWidget
{
    Q_OBJECT
public:
    enum DataRestoreState
    {
        IDEL = 0,  // 空闲
        CHECKING,  // 环境校验中
        RESTORING  // 还原中
    };

    enum DataRestorePage
    {
        HOME_PAGE,        // 首页
        CHECK_ENV_PAGE,   // 环境检测页
        RESTORE_PAGE,      // 还原中页
        LAST_PAGE,        // 结束页
    };

    DECLARE_DYNCREATE_WITH_ONE_PARAM(DataRestore, QStackedWidget*, QWidget*)

public:
    explicit DataRestore(QWidget *parent = nullptr);
    virtual ~DataRestore();

private:
    void initHomePage();
    void initCheckPage();
    void initRestoringPage();
    void initLastPage();

    bool checkIsNeedReboot();

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
    void reboot();

private:
    // dbus接口
    ComKylinBackupManagerInterface *m_pInterface;

    QString m_uuid; // 还原点的UUID
    QString m_devPath; // 如果是从移动设备进行还原，此中保存移动设备挂载路径
    // 本地备份还是U盘备份: 0-本地备份；1-移动设备备份；2-异机备份（仅用于业务场景标记，不用于持久化记录）；3-自定义路径备份
    int m_iPosition;
    // 系统备份状态
    int m_dataRestoreState;
    // 是否需要重启
    bool m_bNeedReboot;
};

#endif // DATARESTORE_H
