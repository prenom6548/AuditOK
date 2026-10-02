#ifndef GHOSTIMAGE_H
#define GHOSTIMAGE_H

#include <QStackedWidget>
#include "udiskdetector.h"
#include "../backup_manager_interface.h"
#include "../backup-daemon/parsebackuplist.h"
#include "../component/backuplistwidget.h"
#include "../../common/dynamiccreator.h"

class GhostImage : public QStackedWidget
{
    Q_OBJECT
public:
    enum GhostImageState
    {
        IDEL = 0,  // 空闲
        CHECKING,  // 环境校验中
        GHOSTING  // 镜像制作中
    };

    enum GhostImagePage
    {
        HOME_PAGE,        // 首页
        SELECT_PATH_PAGE, // 选择镜像保存路径页
        CHECK_ENV_PAGE,   // 环境检测页
        GHOSTING_PAGE,      // 备份中页
        LAST_PAGE,        // 结束页
    };

    DECLARE_DYNCREATE_WITH_ONE_PARAM(GhostImage, QStackedWidget*, QWidget*)

public:
    explicit GhostImage(QWidget *parent = nullptr);
    virtual ~GhostImage();

private:
    void initHomePage();
    void initSetSavePathPage();
    void initCheckPage();
    void initGhostingPage();
    void initLastPage();
    QString createGhostImageName();

signals:
    void startCheckEnv();
    void checkEnvResult(bool result, const QString &errMsg = "", const QString &errTip = "");
    void startGhost();
    void progress(int state, int rate);
    void checkGhostResult(bool result, const QString &errMsg = "", const QString &errTip = "");
    void backupWarnning(const QString &warnning);

public slots:
    void on_pre_clicked(bool checked = false);
    void on_next_clicked(bool checked = false);
    void on_checkEnv_start();
    void on_checkEnv_end(int result);
    void on_ghost_start();
    void on_checkGhost_end(int result);
    void on_ghost_end(bool result);

private:

    // U盘探测
    UdiskDetector* m_udector;
    // U盘挂载路径列表
    QStringList m_udiskPaths;
    // 是否本地保存镜像
    bool m_isLocal;
    // 镜像状态
    int m_ghostImageState;
    // 增量备份选择的备份点uuid
    QString m_uuid;
    // 选中的备份目标路径前缀（暂指udisk挂载路径）
    QString m_prefixDestPath;
    // 本地备份还是U盘备份: 0-本地备份；1-移动设备备份；2-异机备份（仅用于业务场景标记，不用于持久化记录）；3-自定义路径备份
    int m_iPosition;
    // dbus接口
    ComKylinBackupManagerInterface *m_pInterface;
    // 备份点名称
    QString m_backupName;
};

#endif // GHOSTIMAGE_H
