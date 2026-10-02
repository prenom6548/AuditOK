#include "globaldata.h"
#include <QVariant>
#include <QDir>
#include <QDebug>
#include <QPushButton>
#include <QStandardPaths>
#include "kom-configure.h"
#include "kom-buriedpoint.h"
#include "dbus_args.h"

QString GlobalData::g_appNameText;
QIcon GlobalData::g_appNameIconText;

QString GlobalData::g_textMainWidgetWelcome;
QString GlobalData::g_textMainWidgetInfo;
QString GlobalData::g_textMigrationTypeApp;
QString GlobalData::g_textMigrationTypeUser;
QString GlobalData::g_textMigrationTypeSystem;
QIcon GlobalData::g_iconMigrationTypeApp;
QIcon GlobalData::g_iconMigrationTypeHome;
QIcon GlobalData::g_iconMigrationTypeSys;
QString GlobalData::g_textStartBtn;
QString GlobalData::g_textTipBtn;

QString GlobalData::g_textChooseDeviceType;
QString GlobalData::g_textOldSystem;
QString GlobalData::g_textOldSystemNote;
QString GlobalData::g_textNewSystem;
QString GlobalData::g_textNewSystemNote;
QString GlobalData::g_textNextBtn;

QString GlobalData::g_textChooseOldSysType;
QString GlobalData::g_textLinuxSystem;
QString GlobalData::g_textLinuxSystemNote;
QString GlobalData::g_textWinSystem;
QString GlobalData::g_textWinSystemNote;
QString GlobalData::g_textBackBtn;

QString GlobalData::g_textOldSysInfo;
QString GlobalData::g_textOldSysSignage;
QString GlobalData::g_textOldSysUserName;
QString GlobalData::g_textOldSysCheckCode;
QString GlobalData::g_textBtnStartSharing;
QString GlobalData::g_textBtnSharing;
QString GlobalData::g_textTip;

QString GlobalData::g_textReceTabPair; // 配对连接
QString GlobalData::g_textReceTabTipPair; // 选择旧设备
QString GlobalData::g_textReceTabRange; // 选择迁移范围
QString GlobalData::g_textReceTabTipRange; // 选择要传输至新设备的数据范围
QString GlobalData::g_textReceTabMigrate; // 迁移数据
QString GlobalData::g_textReceTabTipMigrate; // 正在迁移数据，请保持新旧设备处于连接状态
QString GlobalData::g_textReceTabComplete; // 迁移完成
QString GlobalData::g_textReceTabTipComplete; // 迁移成功
    // 1
QString GlobalData::g_textReceOldInfo; // 请输入旧设备信息
QString GlobalData::g_textReceChooseOld; // 请选择您的旧设备
QString GlobalData::g_textRectRequired; //必填项
QString GlobalData::g_textRectTipCode; // 校验码错误
    // 2
QString GlobalData::g_textReceMigrateApp; // 应用迁移
QString GlobalData::g_textReceMigrateUser; // 用户数据迁移
QString GlobalData::g_textRectMigrateSys; // 系统配置迁移
QString GlobalData::g_textRectMigrate; // 开始迁移
QString GlobalData::g_textRectSelected; // 已选择：
QString GlobalData::g_textRectCalculating; // 计算中...
QString GlobalData::g_textRectTipCalculating; // 数据正在计算中，请稍等
QString GlobalData::g_textRectTipApp; // 新设备提供兼容环境，让旧设备的第三方应用可以在新系统中运行
QString GlobalData::g_textRectTipUser; // 旧设备home目录和data目录下的所有文件， 迁移到新设备的对应目录
QString GlobalData::g_textRectTipSys; // 将旧设备etc目录下的系统配置文件迁移到新设备
QString GlobalData::g_textRectChooseApp; // 将旧设备etc目录下的系统配置文件迁移到新设备
    // 3
QString GlobalData::g_textRectMigrateTip; // 请确保已连接电源，请勿合盖、运行其他程序
QString GlobalData::g_textReceMigrateOld; // 旧设备
QString GlobalData::g_textReceMigrateSending; // 正在发送
QString GlobalData::g_textRectMigrateNew; // 新设备
QString GlobalData::g_textRectMigrateCurrent;// 当前设备
QString GlobalData::g_textRectMigrateCannel; // 取消迁移
QString GlobalData::g_textRectMigrateSucc; // 迁移成功！
QString GlobalData::g_textRectMigrateConfigSucc; // 配置成功
    // 4
QString GlobalData::g_textRectMigrateSize; // 总计迁移数据  300G 
QString GlobalData::g_textRectMigrateTime; // 迁移耗时 2分45秒
QString GlobalData::g_textRectReboot; // 立即重启
QString GlobalData::g_textRectTipReboot; // 迁移数据将在系统重启后生效
QString GlobalData::g_textRectMsgReboot; // 迁移数据将在系统重启后生效，是否立即重启？

    // 异常处理弹窗
QString GlobalData::g_textRectMsgNetErr1; // 网络故障，迁移失败，请检查网络连接情况。
QString GlobalData::g_textRectMsgNetErr2; // 网络故障，迁移失败，已迁移的数据将被清空，请检查网络连接情况。
QString GlobalData::g_textRectMsgNetErr3; // 网络异常，请检查网络状态！
QString GlobalData::g_textRectMsgCodeErr; // 旧设备密码错误，请重新配对！
QString GlobalData::g_textRectMsgCannelMigrate; // 取消迁移，已迁移的数据将被清空，是否应用操作？
QString GlobalData::g_textRectMsgSure; // 确定
QString GlobalData::g_textRectMsgCannel; // 取消

QIcon GlobalData::g_iconNewSys;
QIcon GlobalData::g_iconOldSys;
QIcon GlobalData::g_iconWinSys;
QIcon GlobalData::g_iconLinuxSys;
QIcon GlobalData::g_iconWarning;
QIcon GlobalData::g_iconSending;
QIcon GlobalData::g_iconTips;
QIcon GlobalData::g_iconStuta;

QSize GlobalData::g_sizeRadioWidget;
QSize GlobalData::g_sizeRadioRectWidget;
QSize GlobalData::g_sizeRadioBtn;
QSize GlobalData::g_sizeDeviceIconBig;
QSize GlobalData::g_sizeDeviceIcon;
QSize GlobalData::g_sizeMainWidget;
QSize GlobalData::g_sizeWidget;

GlobalData::GlobalData()
{
    initText();

    m_dbus = new QDBusInterface(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_DATA_MIGRATION, dbus::DBUS_INTERFACE_DATA_MIGRATION,
                                QDBusConnection::systemBus());
}

GlobalData *GlobalData::getInstance()
{
    static GlobalData globalData;
    return &globalData;
}
GlobalData::~GlobalData()
{
    m_dbus->deleteLater();
}

void GlobalData::initText()
{
    g_appNameIconText = QIcon::fromTheme("kylin-os-manager-remote-tech");
    g_appNameText = tr("Replacement Assistant");
    /* 主界面内容 */
    g_textMainWidgetWelcome = tr("Welcome to the replacement assistant"); // 欢迎使用换机助手
    g_textMainWidgetInfo = tr("Quickly migrate applications, user data, and system config to new computers");// 快速把应用、数据、系统配置迁移至新电脑
    g_textMigrationTypeApp = tr("Application");// 应用迁移 Migration
    g_textMigrationTypeUser = tr("User Data");// 用户数据迁移 Migration
    g_textMigrationTypeSystem = tr("System Config");// 系统配置迁移 Migration
    g_iconMigrationTypeApp = QIcon(":/res/appIcon_light.png");// 应用迁移
    g_iconMigrationTypeHome = QIcon(":/res/useIcon_light.png");// 用户数据迁移
    g_iconMigrationTypeSys = QIcon(":/res/sysIcon_light.png");// 系统配置迁移

    g_textStartBtn = tr("Start");// 开始使用
    g_textTipBtn = tr("How to use the replacement assistant");// 如何使用换机助手
    /* 选择设备界面 */
    g_textChooseDeviceType = tr("Select the device being used"); // 您正在使用的设备是
    g_textOldSystem = tr("Old Device"); // 旧设备 device
    g_textOldSystemNote = tr("Send Data"); // 发送数据
    g_textNewSystem = tr("New Device"); // 新设备
    g_textNewSystemNote = tr("Receive Data"); // 接收数据
    g_textNextBtn = tr("Next Step"); // 下一步
    /* 旧设备系统选择界面 */
    g_textChooseOldSysType = tr("Please select the old device type"); // 请选择旧设备类型
    g_textLinuxSystem = tr("Kylin, Openkylin, and other Linux systems"); // 银河麒麟、openkylin及其他linux发行版
    g_textLinuxSystemNote = tr("Support application, user data and system config migration"); // 支持将应用、数据、系统配置迁移至新设备
    g_textWinSystem = tr("windows system"); // windows系统
    g_textWinSystemNote = tr("Support data migration"); // 支持将数据迁移至新设备
    g_textBackBtn = tr("Back Step"); // 上一步

    /* 旧设备信息界面 */
    g_textOldSysInfo = tr("Fill in old device information");// 请输入旧设备信息
    g_textOldSysSignage = tr("Signage");// 设备标识
    g_textOldSysUserName = tr("User Name");// 用户名
    g_textOldSysCheckCode = tr("Check Code");// 校验码
    g_textBtnStartSharing = tr("Share");// 开启共享
    g_textBtnSharing = tr("Sharing...");// 共享中...
    g_textTip = tr("Please do not enter special characters");

    /* 新设备迁移界面 */
    g_textReceTabPair = tr("Pair"); // 配对连接
    g_textReceTabTipPair = tr("Select old device"); // 选择旧设备
    g_textReceTabRange = tr("Range"); // 选择迁移范围
    g_textReceTabTipRange = tr("Select range to transfer to new device"); // 选择要传输至新设备的数据范围
    g_textReceTabMigrate = tr("Migrate"); // 迁移数据
    g_textReceTabTipMigrate = tr("Migrating data, please keep new and old devices connected"); // 正在迁移数据，请保持新旧设备处于连接状态
    g_textReceTabComplete = tr("Complete"); // 迁移完成
    g_textReceTabTipComplete = tr("Migration successful"); // 迁移成功
    /*1*/
    g_textReceOldInfo = g_textOldSysInfo; // 请输入旧设备信息
    g_textReceChooseOld = tr("Please select your old device"); // 请选择您的旧设备
    g_textRectRequired = tr("Required"); // 必填项
    g_textRectTipCode = tr("code error"); // 校验码错误
    /*2*/
    g_textReceMigrateApp = g_textMigrationTypeApp; // 应用迁移
    g_textReceMigrateUser = g_textMigrationTypeUser; // 用户数据迁移
    g_textRectMigrateSys = g_textMigrationTypeSystem; // 系统配置迁移
    g_textRectMigrate = tr("Migration"); // 开始迁移
    g_textRectSelected = tr("Selected:"); // 已选择：
    g_textRectCalculating = tr("Calculating..."); // 计算中...
    g_textRectTipCalculating = tr("Data is being calculated, please wait"); // 数据正在计算中，请稍等
    g_textRectTipApp = tr("The new device provides a compatible environment, allowing third-party applications from the old device to run in the new system"); // 新设备提供兼容环境，让旧设备的第三方应用可以在新系统中运行
    g_textRectTipUser = tr("Migrate all files in the home and data directories of the old device to the corresponding directory of the new device"); // 旧设备home目录和data目录下的所有文件， 迁移到新设备的对应目录
    g_textRectTipSys = tr("Migrate the system configuration files from the old device etc directory to the new device"); // 将旧设备etc目录下的系统配置文件迁移到新设备
    g_textRectChooseApp = tr("Select application:"); // 选择应用：

    // 3
    g_textRectMigrateTip = tr("Please ensure that the power supply is connected and do not close the cover or run other programs"); // 请确保已连接电源，请勿合盖、运行其他程序
    g_textReceMigrateOld = g_textOldSystem; // 旧设备
    g_textReceMigrateSending = tr("Sending"); // 正在发送
    g_textRectMigrateNew = g_textNewSystem; // 新设备
    g_textRectMigrateCurrent = tr("Current device");// 当前设备
    g_textRectMigrateCannel = tr("Cancel Migration"); // 取消迁移
    g_textRectMigrateSucc = tr("Migration successful!"); // 迁移成功！
    g_textRectMigrateConfigSucc = tr("Successfully configured"); // 配置成功

    // 4
    g_textRectMigrateSize = tr("Migration size"); // 总计迁移数据  300G 
    g_textRectMigrateTime = tr("Migration duration"); // 迁移耗时 2分45秒
    g_textRectReboot = tr("Reboot"); // 立即重启
    g_textRectTipReboot = tr("Migration data will take effect after system restart"); // 迁移数据将在系统重启后生效
    g_textRectMsgReboot = tr("The migration data will take effect after the system restarts. Do you want to restart now?"); // 迁移数据将在系统重启后生效，是否立即重启？

    // 异常处理弹窗
    g_textRectMsgNetErr1 = tr("Network failure, migration failed. Please check the network connection."); // 网络故障，迁移失败，请检查网络连接情况。
    g_textRectMsgNetErr2 = tr("Network failure, migration failed, migrated data will be cleared, please check network connectivity."); // 网络故障，迁移失败，已迁移的数据将被清空，请检查网络连接情况。
    g_textRectMsgNetErr3 = tr("Network abnormality, please check the network status!"); // 网络异常，请检查网络状态！
    g_textRectMsgCodeErr = tr("Old device password error, please pair again!"); // 旧设备密码错误，请重新配对！
    g_textRectMsgCannelMigrate = tr("Canceling migration will clear the migrated data. Do you want to apply the action?"); // 取消迁移，已迁移的数据将被清空，是否应用操作？
    g_textRectMsgSure = tr("Ok"); // 确定
    g_textRectMsgCannel = tr("Cancel"); // 取消

    g_iconNewSys = QIcon(":/res/new_sys_icon.png");
    g_iconOldSys = QIcon(":/res/old_sys_icon.png");
    g_iconWinSys = QIcon(":/res/win_sys_icon.png");
    g_iconLinuxSys = QIcon(":/res/linux_sys_icon.png");
    g_iconWarning = QIcon(":/res/warning.svg");
    g_iconSending = QIcon(":/res/sending_icon.svg");
    g_iconTips = QIcon(":/res/tip_icon.svg");
    g_iconStuta = QIcon(":/res/status_icon.svg");

    g_sizeRadioWidget = QSize(254, 264);
    g_sizeRadioRectWidget = QSize(524, 98);
    g_sizeRadioBtn = QSize(16, 16);
    g_sizeDeviceIconBig = QSize(144, 144);
    g_sizeDeviceIcon = QSize(66, 66);
    g_sizeMainWidget = QSize(904, 568);
    g_sizeWidget = QSize(904, 520);
}

void GlobalData::setHighColorBtn(QPushButton *btn)
{
    btn->setProperty("isImportant", true);
    btn->setFlat(false);
}

void GlobalData::dbusSenderStartServer(QString name, QString pwd)
{
    QDBusMessage msg = m_dbus->call("senderStartServer", name, pwd);
    if (msg.errorMessage() != "") {
        qDebug() << "dbus interface error : senderStartServer" << msg.errorMessage();
    }
}

void GlobalData::dbusSenderStopServer()
{
    QDBusMessage msg = m_dbus->call("senderStopServer");
    if (msg.errorMessage() != "") {
        qDebug() << "dbus interface error : senderStopServer" << msg.errorMessage();
    }
}

void GlobalData::dbusSenderGetDesktopInfo()
{
    QDBusMessage msg = m_dbus->call("senderGetAppDesktopInfo");
    if (msg.errorMessage() != "") {
        qDebug() << "dbus interface error : senderStopServer" << msg.errorMessage();
    }
}

void GlobalData::dbusReceiverTestServer(QString pwd, QString IP, QString name)
{
    QDBusMessage msg = m_dbus->call("receiverTestServer", pwd, IP, name);
    if (msg.errorMessage() != "") {
        qDebug() << "dbus interface error : receiverTestServer " << msg.errorMessage();
    }
}

void GlobalData::dbusReceiverMigrateData(ChooseDataType type)
{
    QDBusMessage msg = m_dbus->call("receiverMigrateData", type);
    if (msg.errorMessage() != "") {
        qDebug() << "dbus interface error : receiverMigrateData" << msg.errorMessage();
    }
}

void GlobalData::dbusReceiverStopRsync()
{
    QDBusMessage msg = m_dbus->call("receiverStopRsync");
    if (msg.errorMessage() != "") {
        qDebug() << "dbus interface error : receiverStopRsync" << msg.errorMessage();
    }
}

void GlobalData::dbusReceiverReboot()
{
    QDBusMessage msg = m_dbus->call("receiverReboot");
    if (msg.errorMessage() != "") {
        qDebug() << "dbus interface error : receiverReboot" << msg.errorMessage();
    }
}

void GlobalData::dbusReceiverGetDataInfo(ChooseDataType type)
{
    QDBusMessage msg = m_dbus->call("receiverGetDataInfo", type);
    if (msg.errorMessage() != "") {
        qDebug() << "dbus interface error : receiverGetDataInfo" << msg.errorMessage();
    }
}