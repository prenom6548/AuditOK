#ifndef DBUS_H
#define DBUS_H

#include <QThread>
#include <QProcess>
#include "dbus_args.h"
namespace dbus
{

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
public slots:
    /* 发送端 */
    void senderStartServer(QString name, QString pwd) ; // 开启服务
    void senderStopServer(); // 关闭服务, 使用命令killall rsync ，删去配置文件
    /* 接收端 */
    void receiverTestServer(QString pwd, QString IP, QString name); // 创建用户认证，测试连接
    void receiverGetDataInfo(int type); // 获取迁移数据大小
    void receiverMigrateData(int type); // 迁移数据
    void receiverReboot();
    void receiverAppDataList(QStringList list);
    void receiverConfigData();//迁移完成后进行配置
    void receiverStopRsync(); // 停止文件迁移
private:
    void senderCreatConfigFile(QString name); // 创建rsync的配置文件，
    void senderCreatPwdFile(QString name, QString pwd); // 创建用户认证文件

    void receiverCreatPwdFile(QString pwd); // 生成用户认证文件
    void receiverSendSuccSig(); // 接收端测试连接，发出成功信号
    int receiverError(QString error);
    void processDataSizeOut(); // 接收端拉取数据大小
    void processMrigrationOut(); // 接收端拉取数据
    void configSysData();
    void configUseData();
    void configAppData();
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
    QString m_sendConfigFile = "/etc/rsyncd.conf";  // rsync配置文件
    QString m_sendConfigFileBackup = "/etc/rsyncd.conf_backup"; // 配置文件备份
    QString m_sendPwdFile = "/etc/kylinReplacementSender.passwd"; // 发送端认证文件
    QString m_recePwdFile = "/tmp/kylinReplacementReceiver.passwd"; // 接收端认证文件
    QString m_excludeFile = "/usr/share/kylin-os-manager/replacement/data/ReplacementDataExcludeFile"; // 黑名单
    QString m_excludeHomeAndEtcFile = "/usr/share/kylin-os-manager/replacement/data/mrigationHomeAndEtcFile.list"; // 黑名单
    QString m_appListFile = "/usr/share/kylin-os-manager/replacement/destktop.list"; // 被选中应用列表
    QString m_appDataYamlFile = "/usr/share/kylin-os-manager/replacement/data/appClassification.yaml"; // app的列表
    QString m_configDataYamlFile = "/usr/share/kylin-os-manager/replacement/data/etcPolicy.yaml"; // etc的列表
    QString m_receiverDataFile = "/fs.old";// 接受数据目录 todo:后续会读取发送端的类型，标为目录名
    QProcess *m_processTest = nullptr;
    QProcess *m_processDataSize = nullptr;
    QProcess *m_processMigrateDate = nullptr;
    
signals:
    void sigReceTestSuccess();
    void sigReceProcessFail(int);
    void sigMigrateDataInfo(QStringList);
    void sigMigrateDataSize(qint64);
    void sigMigrateDataSuccess();
    void sigConfigDataSuccess();
};

} // namespace dbus

#endif // DBUS_H
