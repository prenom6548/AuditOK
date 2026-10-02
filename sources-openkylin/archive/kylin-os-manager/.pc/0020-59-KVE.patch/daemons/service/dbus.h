#ifndef DBUS_H
#define DBUS_H

#include <QThread>
#include <QProcess>
#include <QtDBus/QDBusObjectPath>
#include "dbus_args.h"
namespace dbus
{
    #define SYSTEMD_SERVICE         "org.freedesktop.login1"
    #define SYSTEMD_PATH            "/org/freedesktop/login1"
    #define SYSTEMD_INTERFACE       "org.freedesktop.login1.Manager"
class Tool : public QObject
{
    Q_OBJECT
public:
    Tool(QObject *parent = nullptr) : QObject(parent) {}
public slots:
    QStringList getFileNameFromDir(const QString &arg) const;
};

class Dbus : public QObject
{
    Q_OBJECT
public:
    Dbus(QObject *parent = nullptr) : QObject(parent) {}
public slots:
    void creatDbusServer();
};

class DbusManager : public QObject
{
    Q_OBJECT
signals:
    void creatDbusServer();

public:
    DbusManager();
};

class DataMigration : public QObject
{
    Q_OBJECT
public:
    DataMigration(QObject *parent = nullptr);

    enum MigrateDataType {
        AppData = 0,
        UserData,
        SysConfData,
        AppAndUserData,
        AppAndSysData,
        UserAndSysData,
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

    struct UseInfo {
        quint32 uid;
        QString name;
        QDBusObjectPath path;
    };
    typedef QList<UseInfo> UseInfoList;

public slots:
    /* 发送端 */
    void senderStartServer(QString name, QString pwd) ; // 开启服务
    void senderStopServer(); // 关闭服务, 使用命令killall rsync ，删去配置文件
    void senderGetAppDesktopInfo(); // 获取本机应用的信息
    void senderGetAppPackageInfo(QStringList list); // 获取本机应用包的信息

    /* 接收端 */
    QString receiverTestServer(QString pwd, QString IP, QString name); // 创建用户认证，测试连接
    void receiverGetDataInfo(int type); // 获取迁移数据大小
    void receiverMigrateData(int type); // 迁移数据
    void receiverReboot();
    void receiverAppDataList(QStringList list);
    void receiverConfigData();//迁移完成后进行配置
    void receiverStopRsync(); // 停止文件迁移
    void receiverMigrateVersionFile(); // 迁移前获取系统类别
    void receiverMigrateAppData(); // 应用desktop迁移
private:
    void senderCreatConfigFile(QString name); // 创建rsync的配置文件，
    void senderCreatPwdFile(QString name, QString pwd); // 创建用户认证文件
    void senderGetVersion();
    void senderGetUserInfo(); // 发送端获取本机信息（用户信息id）

    void receiverCreatPwdFile(QString pwd); // 生成用户认证文件
    void receiverSendSuccSig(); // 接收端测试连接，发出成功信号
    int receiverError(QString error);
    void processDataSizeOut(); // 接收端拉取数据大小
    void processMrigrationOut(); // 接收端拉取数据
    void processDesktopToPackageOut(); // 发送端获取到使用dpkg -S获取到的数据
    void processPackageInfoOut(); // 发送端获取到使用dpkg -l获取到的数据
    void configSysData();
    void configUseData();
    void configAppData();
    void appDesktopToPkg(QStringList list);
    QStringList readYamlFile(QString str);
    QStringList getHomeDirChildDir(QString str);
    QStringList getUserInfo(QString uname, QString filePath);
    bool NewSysExistUid(int id);
    void updateNewSysEtcFile(QString file, QStringList list, int oldUid, int newUid);
    // QStringList findIcon(QString path, QString name);
    void startKare();

    QString m_ip;
    QString m_name;
    bool m_senderConfigFileIsExesit = false;
    int m_type;
    QStringList m_listFile;
    QStringList m_listFolders;
    QStringList m_desktopFileIsRightList;
    QStringList m_packsgeNameList;
    QHash <QString, QStringList> m_hashPackage;
    QString m_sendConfigFile = "/etc/rsyncd.conf";  // rsync配置文件
    QString m_sendConfigFileBackup = "/etc/rsyncd.conf_backup"; // 配置文件备份
    QString m_sendPwdFile = "/etc/kylinReplacementSender.passwd"; // 发送端认证文件
    QString m_recePwdFile = "/tmp/kylinReplacementReceiver.passwd"; // 接收端认证文件
    QString m_excludeFile = "/usr/share/kylin-os-manager/replacement/data/ReplacementDataExcludeFile"; // 黑名单
    QString m_excludeHomeAndEtcFile = "/usr/share/kylin-os-manager/replacement/data/mrigationHomeAndEtcFile.list"; // 黑名单

    QString m_appListFile = "/usr/share/kylin-os-manager/replacement/desktop.list"; // 被选中应用列表
    QString m_appInfoListFile = "/usr/share/kylin-os-manager/replacement/package-desktop.info"; // 被选中的应用列表

    QString m_senderappInfoListFile = "/usr/share/applications/package-desktop.info"; // 应用列表信息
    QString m_senderappListFile = "/usr/share/applications/replacement_destktop.list"; // 可迁移应用列表
    QString m_senderDataDir = "/tmp/komt/"; // 存放【系统类别、用户信息】文件夹
    QString m_senderVersion = "/tmp/komt/komtReplacement"; // 系统版本
    QString m_senderUserInfo = "/tmp/komt/komtUserInfo"; // 使用dbus获取到的用户信息数据

    QString m_appDataYamlFile = "/usr/share/kylin-os-manager/replacement/data/appClassification.yaml"; // app的列表
    QString m_configDataYamlFile = "/usr/share/kylin-os-manager/replacement/data/etcPolicy.yaml"; // etc的列表
    QString m_receiverDataFile = "/fs.old";// 接受数据目录 todo:后续会读取发送端的类型，标为目录名
    QString m_receiverFilePathName = "fs.old";// 接受数据目录 todo:后续会读取发送端的类型，标为目录名
    QString desktopFilePath = "/usr/share/applications/";
    QString DesktopGroup = "Desktop Entry";
    QString DesktopKeyNameEN = "Name";
    QString DesktopKeyNameZH = "Name[zh_CN]";
    QString DesktopKeyCommentEN = "Comment";
    QString DesktopKeyCommentZH = "Comment[zh_CN]";
    QProcess *m_processTest = nullptr;
    QProcess *m_processDataSize = nullptr;
    QProcess *m_processMigrateDate = nullptr;
    QProcess *m_processMigrateDektopDate = nullptr;
    QProcess *m_processGetDesktopInfo = nullptr;
    QProcess *m_processGetPackageInfo = nullptr;
    int num = 0;

signals:
    void sigReceTestSuccess();
    void sigReceProcessFail(int);
    void sigMigrateDataInfo(QStringList);
    void sigMigrateDataSize(qint64);
    void sigMigrateDataSuccess();
    void sigConfigDataSuccess();
    void sigMigrateAppDataSuccess();
};

} // namespace dbus

Q_DECLARE_METATYPE(dbus::DataMigration::UseInfo)
Q_DECLARE_METATYPE(dbus::DataMigration::UseInfoList)

#endif // DBUS_H
