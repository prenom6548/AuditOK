#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include <QObject>
#include <QIcon>
#include <QPushButton>
#include <QDateTime>
#include <QDBusInterface>

/* 错误类型 */
enum SysType {
    NewSys = 0,
    OldSys,
    WinSys,
    LinuxSys,
    NobadySys,
};

enum ChooseDataType {
    AppData = 0,
    UserData,
    SysData,
    AppAndUser,
    AppAndSys,
    UserAndSys,
    AllData,
};

/* 错误类型 */
enum ErrorTypeEnum {
    ErrorConnectFailed = 0, /* No route to host */
    ErrorRsyncCommand,      /* The remote path must start with a module name not */
    ErrorRsyncPassWord,     /* auth failed on module */
    ErrorRsyncServerConfig, /* Unknown module */
    ErrorServerNotStart,    /* Connection refused */
    ErrorFileTransfer,      /* receiving incremental file list */
    ErrorNormal,
};

class GlobalData : public QObject
{
    Q_OBJECT
public:
    static QString g_appNameText;
    static QIcon g_appNameIconText;

    static QString g_textMainWidgetWelcome;
    static QString g_textMainWidgetInfo;

    static QString g_textMigrationTypeApp;
    static QString g_textMigrationTypeUser;
    static QString g_textMigrationTypeSystem;
    static QIcon g_iconMigrationTypeApp;
    static QIcon g_iconMigrationTypeHome;
    static QIcon g_iconMigrationTypeSys;
    static QString g_textStartBtn;
    static QString g_textTipBtn;

    static QString g_textChooseDeviceType;
    static QString g_textOldSystem;
    static QString g_textOldSystemNote;
    static QString g_textNewSystem;
    static QString g_textNewSystemNote;
    static QString g_textNextBtn;

    static QString  g_textChooseOldSysType; // 请选择旧设备类型
    static QString  g_textLinuxSystem; // 银河麒麟、openkylin及其他linux发行版
    static QString  g_textLinuxSystemNote; // 支持将应用、数据、系统配置迁移至新设备
    static QString  g_textWinSystem; // windows系统
    static QString  g_textWinSystemNote; // 支持将数据迁移至新设备
    static QString  g_textBackBtn; // 上一步

    static QString g_textOldSysInfo;
    static QString g_textOldSysSignage;
    static QString g_textOldSysUserName;
    static QString g_textOldSysCheckCode;
    static QString g_textBtnStartSharing;
    static QString g_textBtnSharing;
    static QString g_textTip;

    static QString g_textReceTabPair; // 配对连接
    static QString g_textReceTabTipPair; // 选择旧设备
    static QString g_textReceTabRange; // 选择迁移范围
    static QString g_textReceTabTipRange; // 选择要传输至新设备的数据范围
    static QString g_textReceTabMigrate; // 迁移数据
    static QString g_textReceTabTipMigrate; // 正在迁移数据，请保持新旧设备处于连接状态
    static QString g_textReceTabComplete; // 迁移完成
    static QString g_textReceTabTipComplete; // 迁移成功
    // 1
    static QString g_textReceOldInfo; // 请输入旧设备信息
    static QString g_textReceChooseOld; // 请选择您的旧设备
    static QString g_textRectRequired; //必填项
    static QString g_textRectTipCode; // 校验码错误
    // 2
    static QString g_textReceMigrateApp; // 应用迁移
    static QString g_textReceMigrateUser; // 用户数据迁移
    static QString g_textRectMigrateSys; // 系统配置迁移
    static QString g_textRectMigrate; // 开始迁移
    static QString g_textRectSelected; // 已选择：
    static QString g_textRectCalculating; // 计算中...
    static QString g_textRectTipCalculating; // 数据正在计算中，请稍等
    static QString g_textRectTipApp; // 新设备提供兼容环境，让旧设备的第三方应用可以在新系统中运行
    static QString g_textRectTipUser; // 旧设备home目录和data目录下的所有文件， 迁移到新设备的对应目录
    static QString g_textRectTipSys; // 将旧设备etc目录下的系统配置文件迁移到新设备
    static QString g_textRectChooseApp; // 选择应用：
    // 3
    static QString g_textRectMigrateTip; // 请确保已连接电源，请勿合盖、运行其他程序
    static QString g_textReceMigrateOld; // 旧设备
    static QString g_textReceMigrateSending; // 正在发送
    static QString g_textRectMigrateNew; // 新设备
    static QString g_textRectMigrateCurrent;// 当前设备
    static QString g_textRectMigrateCannel; // 取消迁移
    static QString g_textRectMigrateSucc; // 迁移成功！
    static QString g_textRectMigrateConfigSucc; // 配置成功
    // 4
    static QString g_textRectMigrateSize; // 总计迁移数据  300G 
    static QString g_textRectMigrateTime; // 迁移耗时 2分45秒
    static QString g_textRectReboot; // 立即重启
    static QString g_textRectTipReboot; // 迁移数据将在系统重启后生效
    static QString g_textRectMsgReboot; // 迁移数据将在系统重启后生效，是否立即重启？

    // 异常处理弹窗
    static QString g_textRectMsgNetErr1; // 网络故障，迁移失败，请检查网络连接情况。
    static QString g_textRectMsgNetErr2; // 网络故障，迁移失败，已迁移的数据将被清空，请检查网络连接情况。
    static QString g_textRectMsgNetErr3; // 网络异常，请检查网络状态！
    static QString g_textRectMsgCodeErr; // 旧设备密码错误，请重新配对！
    static QString g_textRectMsgCannelMigrate; // 取消迁移，已迁移的数据将被清空，是否应用操作？
    static QString g_textRectMsgSure; // 确定
    static QString g_textRectMsgCannel; // 取消

    static QIcon g_iconOldSys;
    static QIcon g_iconNewSys;
    static QIcon g_iconWinSys;
    static QIcon g_iconLinuxSys;
    static QIcon g_iconWarning;
    static QIcon g_iconSending;
    static QIcon g_iconTips;
    static QIcon g_iconStuta;

    static QSize g_sizeRadioWidget;
    static QSize g_sizeRadioRectWidget;
    static QSize g_sizeRadioBtn;
    static QSize g_sizeDeviceIconBig;
    static QSize g_sizeDeviceIcon;
    static QSize g_sizeMainWidget;
    static QSize g_sizeWidget;

    static GlobalData *getInstance();
    void initText();
    void setHighColorBtn(QPushButton *btn);
    qint64 g_migrationDataSize = 0;
    qint64 g_migrateFinishDataSize = 0;
    QString g_migrateFinishDataStr = "";
    QDateTime g_timeStart;
    QDateTime g_timeEnd;
    QString g_time = "";
    bool g_chooseAppData = true;

    QDBusInterface *m_dbus = nullptr;
    void dbusSenderStartServer(QString name, QString pwd);
    void dbusSenderStopServer();
    void dbusSenderGetDesktopInfo();
    void dbusReceiverTestServer(QString pwd, QString IP, QString name);
    void dbusReceiverMigrateData(ChooseDataType type);
    void dbusReceiverStopRsync();
    void dbusReceiverReboot();
    void dbusReceiverGetDataInfo(ChooseDataType type);

private:
    GlobalData();
    ~GlobalData();

signals:
    void sigFontSizeChange(double fontSize);
};

#endif // GLOBALSIZEDATA_H
