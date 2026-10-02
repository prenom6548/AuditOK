#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusInterface>
#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QTextCodec>
#include "dbus.h"

using namespace dbus;

inline QDBusArgument &operator<<(QDBusArgument &argument, const dbus::DataMigration::UseInfo& useinfo)
{
    argument.beginStructure();
    argument << useinfo.uid;
    argument << useinfo.name;
    argument << useinfo.path;
    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, dbus::DataMigration::UseInfo &useinfo)
{
    argument.beginStructure();
    argument >> useinfo.uid;
    argument >> useinfo.name;
    argument >> useinfo.path;
    argument.endStructure();

    return argument;
}


void Dbus::creatDbusServer()
{
    Tool *toolBus = new Tool(this);
    DataMigration *migrationBus = new DataMigration(this);
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.registerService(DBUS_SERVICENAME)) {
        qDebug() << "creat dbus service error";
    }
    if (!bus.registerObject(DBUS_PARH_TOOL, DBUS_INTERFACE_TOOL, toolBus, QDBusConnection::ExportAllSlots)) {
        qDebug() << "creat dbus interface error" << DBUS_INTERFACE_TOOL;
    }
    if (!bus.registerObject(DBUS_PARH_DATA_MIGRATION, DBUS_INTERFACE_DATA_MIGRATION, migrationBus,
                            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qDebug() << "creat dbus interface data migration error" << DBUS_INTERFACE_DATA_MIGRATION;
    }
}

QStringList Tool::getFileNameFromDir(const QString &arg) const
{
    QStringList tmp;
    QFileInfo info(arg);
    QString path = info.absolutePath();
    tmp = QDir(path).entryList(QStringList() << QString(info.fileName() + "*"), QDir::Files | QDir::Readable, QDir::Name);
    for (QString &str : tmp) {
        str = path + "/" + str;
    }
    return tmp;
}

DbusManager::DbusManager()
{
    QThread *thread = new QThread(this);
    Dbus *dbus = new Dbus();
    connect(this, &DbusManager::creatDbusServer, dbus, &Dbus::creatDbusServer);
    connect(thread, &QThread::finished, dbus, &Dbus::deleteLater);
    dbus->moveToThread(thread);
    thread->start();
    emit creatDbusServer();
}

DataMigration::DataMigration(QObject *parent)
{
    qRegisterMetaType<dbus::DataMigration::UseInfo>("UseInfo");
    qRegisterMetaType<dbus::DataMigration::UseInfoList>("UseInfoList");
    qDBusRegisterMetaType<dbus::DataMigration::UseInfo>();
    qDBusRegisterMetaType<dbus::DataMigration::UseInfoList>();

}

void DataMigration::senderStartServer(QString name, QString pwd)
{
    qDebug() << "name" << name << "pwd" << pwd;
    // 1.创建配置文件
    senderCreatConfigFile(name);
    // 2.生成用户认证文件 
    senderCreatPwdFile(name, pwd);
    // 3.启动
    QProcess process;
    QStringList options;
    options << "rsync --daemon";
    process.start("rsync --daemon");
    process.waitForFinished();
    process.close();
}

void DataMigration::senderStopServer()
{
    QProcess process;
    QStringList options;
    QByteArray res = process.readAllStandardError();
    if (res.isEmpty()) {
        QString deleteFileCom = "rm -rf " + m_sendConfigFile + " " + m_sendPwdFile;
        process.start(deleteFileCom);
        process.waitForFinished();
        if (m_senderConfigFileIsExesit) {
            QString backupFileCom = "mv " + m_sendConfigFileBackup + " " + m_sendConfigFile;
            process.start(backupFileCom);
            process.waitForFinished();
        }
    } else {
        qDebug() << " ===== DBUS === sender killall rsync error:" << res;
    }
    process.start("killall rsync");
    process.waitForFinished();
    process.close();
}

void DataMigration::senderCreatConfigFile(QString name)
{
    // 判断是否存在【/etc/rsyncd.conf】配置文件, 存在 cp 生成备份文件，结束后将其还原
    QFileInfo fileInfo(m_sendConfigFile);
    if (fileInfo.isFile()) { 
        m_senderConfigFileIsExesit = true;

        QString cmd = "cp -rf " + m_sendConfigFile + " " + m_sendConfigFileBackup;
        QString trueCmd = "bash -c \"" + cmd + "\"";
        QProcess proc;
        proc.start(trueCmd);
        proc.waitForFinished();
    }

    QFile file(m_sendConfigFile);
    file.resize(0);
    file.close();
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QString info = "uid = root \n"
                       "gid = root \n"
                       "list = true \n"
                       "[shareFile]\n"
                       "read only = no \n"
                       "write only = no \n"
                       "hostsallow = * \n"
                       "hosts deny = * \n"
                       "ignore errors = true \n"
                       "path = / \n"
                       "auth users = " + name + "\n"
                       "secrets file = " + m_sendPwdFile;
        QByteArray bytes = info.toUtf8();
        int len = file.write(bytes);
        if (len == 0) {
            qDebug() << " ===== DBUS === Sender Config File write data failed !!!";
        }
        len = file.write("\n");
        file.close();

    } else {
        qDebug() << " ===== DBUS === Sender failed to create a new file!";
    }
}

void DataMigration::senderCreatPwdFile(QString name, QString pwd)
{
    QFile file(m_sendPwdFile);
    file.resize(0);
    file.close();
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {

        QString info = name + ":" + pwd;
        QByteArray bytes = info.toUtf8();
        int len = file.write(bytes);
        if (len == 0) {
            qDebug() << " ===== DBUS === Sender PWD File write data failed !!!";
        }
        file.close();

    } else {
        qDebug() << " ===== DBUS === Sender failed to create a new file!";
    }

    QProcess process;
    QStringList options;
    options << "600" << m_sendPwdFile;
    process.start("chmod", options);
    process.waitForFinished();
    process.close();

}

void DataMigration::senderGetVersion()
{
    QString versionStr;
    QFile fileVersion("/etc/issue");
    fileVersion.close();
    if (fileVersion.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QByteArray bytes = fileVersion.readAll();

        versionStr = bytes;

        fileVersion.close();
    } 
    versionStr = versionStr.remove(" ").toLower();

    QFile file(m_senderVersion);
    file.resize(0);
    file.close();
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {

        QByteArray bytes = versionStr.toUtf8();
        int len = file.write(bytes);
        if (len == 0) {
            qDebug() << " ===== DBUS === Receiver PWD File write data failed !!! ===== ";
        }
        file.close();
    } 
}

void DataMigration::senderGetUserInfo()
{
    // QFileInfo fileInfo("/etc/uid_list");
    // if (fileInfo.isFile()) {
    //     return;
    // }

    QProcess process;
    QStringList options;
    options << "-c" << "ls -ln /home | awk  '{if($3>=1000){print $3,$NF}}'";
    process.start("bash", options);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    if (output == "") {
        qDebug() << " ===== Dbus ===== bash -c ls -ln /home 获取信息为空";
        return;
    }
    QStringList lines = output.split("\n");
    if (lines.isEmpty()) {
        qDebug() << " ===== Dbus =====  ls -ln /home 转换成qstringlist为空";
        return;
    }
    QFile file(m_senderUserInfo);
    file.resize(0);
    // 遍历子目录，将目录名存入 QStringList
    foreach (const QString& line, lines) {
        if (line == "") {
            continue;
        }
        qDebug() << line;
        QStringList info = line.split(" ");
        file.close();
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            QString str = info.at(1)  + ":" + info.at(0) + "\n";

            QByteArray bytes = str.toUtf8();
            int len = file.write(bytes);
            if (len == 0) {
                qDebug() << " ===== DBUS === Read Dbus listUsers, File write data failed !!! ===== ";
            }
            file.close();
        } 
    }

    process.close();

    // QDBusInterface interface( SYSTEMD_SERVICE, SYSTEMD_PATH, SYSTEMD_INTERFACE, QDBusConnection::systemBus() );

    // auto message = interface.call("ListUsers");
    // auto arg = message.arguments().at(0);
    // auto a = arg.value<QDBusArgument>();
    // UseInfoList list;
    // a >> list;
    // qDebug() << "+++++++++++++++++" << list.at(0).name;
    // QFile file(m_senderUserInfo);
    // file.resize(0);
    // for (int i = 0; i < list.count(); i++) {
        // file.close();
        // if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)) {

        //     QString str = list.at(i).name  + ":" + QString::number(list.at(i).uid) + "\n";

        //     QByteArray bytes = str.toUtf8();
        //     int len = file.write(bytes);
        //     if (len == 0) {
        //         qDebug() << " ===== DBUS === Read Dbus listUsers, File write data failed !!! ===== ";
        //     }
        //     file.close();
        // } 
    // }

}

QString DataMigration::receiverTestServer(QString pwd, QString ip, QString name)
{
    m_name = name;
    m_ip = ip;
    QString str;
    // 设置用户认证文件
    receiverCreatPwdFile(pwd);

    // 共享列表 rsync --list-only --password-file=/tmp/rsync.passwd name@ip::shareFile;
    if (!m_processTest) {
        m_processTest = new QProcess(this);
        connect(m_processTest, &QProcess::readyReadStandardError, this,[=](){
            QString error = m_processTest->readAllStandardError();
            int errornumm = receiverError(error);
            QDBusMessage send =
                    QDBusMessage::createSignal(DBUS_PARH_DATA_MIGRATION, DBUS_INTERFACE_DATA_MIGRATION, "sigReceProcessFail");
            send << errornumm;
            qDebug() << error;
            QDBusConnection::systemBus().send(send);
        });
        connect(m_processTest, &QProcess::readyReadStandardOutput, this, &DataMigration::receiverSendSuccSig);
    }
    QFile outputFile(m_excludeHomeAndEtcFile);
    outputFile.resize(0);

    QStringList options;
    options << "--list-only"
            << "--password-file=" + m_recePwdFile
            << m_name + "@" + m_ip + "::shareFile";
    m_processTest->start("rsync", options);
    m_processTest->waitForStarted();
    m_processTest->waitForFinished();

    receiverMigrateVersionFile();
    return m_receiverDataFile;
}

void DataMigration::receiverSendSuccSig()
{
    QDBusMessage send =
            QDBusMessage::createSignal(DBUS_PARH_DATA_MIGRATION, DBUS_INTERFACE_DATA_MIGRATION, "sigReceTestSuccess");
    QDBusConnection::systemBus().send(send);

    // 解析标准输出，提取共享目录列表
    QString output = m_processTest->readAllStandardOutput();
    QStringList lines = output.split("\n");
    // 将除了以 "h" 开头的文件或目录名写入文件
    QFile outputFile(m_excludeHomeAndEtcFile);
    outputFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    QTextStream outputStream(&outputFile);
    foreach (const QString& line, lines) {
        QStringList fields = line.split(QRegExp("\\s+"));
        QString entryType = fields.first();
        QString name = fields.last();
        if (!name.startsWith("h") && !name.startsWith("e") && name != "." && name != "") {
            outputStream << name << "/*" << endl;
        }
    }
    outputFile.close();

    // receiverMigrateAppData();
}

int DataMigration::receiverError(QString error)
{
    ErrorTypeEnum type = ErrorTypeEnum::ErrorNormal;
    if (error.contains("No route to host", Qt::CaseSensitive)) {
        qDebug() << " === Error1: 对方没开机、防火墙阻挡 === ";
        type = ErrorTypeEnum::ErrorConnectFailed;
    } else if (error.contains("The remote path must start with a module name not", Qt::CaseSensitive)) {
        qDebug() << " === Error2: rsync命令语法错误 === ";
        type = ErrorTypeEnum::ErrorRsyncCommand;
    } else if (error.contains("auth failed on module", Qt::CaseSensitive)) {
        qDebug() << " === Error3: 用户名和密误填写错误 === ";
        type = ErrorTypeEnum::ErrorRsyncPassWord;
    } else if (error.contains("Unknown module", Qt::CaseSensitive)) {
        qDebug() << " === Error4: 配置文件模块名称书写错误 === ";
        type = ErrorTypeEnum::ErrorRsyncServerConfig;
    } else if (error.contains("Connection refused", Qt::CaseSensitive)) {
        qDebug() << " === Error5: rsync服务没有正确启动 === ";
        type = ErrorTypeEnum::ErrorServerNotStart;
    } else if (error.contains("RsyncConnecting", Qt::CaseSensitive)) {
        qDebug() << " === Error6: 正在同步文件 === ";
        type = ErrorTypeEnum::ErrorNormal;
    }
    return type;
}

void DataMigration::receiverCreatPwdFile(QString pwd)
{
    QFile file(m_recePwdFile);
    file.resize(0);
    file.close();
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {

        QByteArray bytes = pwd.toUtf8();
        int len = file.write(bytes);
        if (len == 0) {
            qDebug() << " ===== DBUS === Receiver PWD File write data failed !!! ===== ";
        }
        file.close();
    } else {
        qDebug()<<" ===== DBUS === failed to create a new file! ===== ";
    }

    QProcess process;
    QStringList options;
    options << "600" << m_recePwdFile;
    process.start("chmod", options);
    process.waitForFinished();
    process.close();
}

void DataMigration::receiverGetDataInfo(int type)
{
    // *
    //  * 应用 : /bin /usr /lib /var /sbin /etc /opt
    //  * 用户 : /home /etc 
    //  * 系统配置 :  /etc 
    //  * 黑名单 : 'var/run/*','backup/*','boot/*','cdrom/*','dev/*','lost+found/*','media/*','mmkv.default','mmkv.default.crc','mnt/*','proc/*','root/*','run/*','srv/*','sys/*','system/*','tmp/*'
    //  *

    if (!m_processDataSize) {
        m_processDataSize = new QProcess(this);
        connect(m_processDataSize, &QProcess::readyReadStandardError, this,[=](){
            QString error = m_processDataSize->readAllStandardError();
            QDBusMessage send =
                    QDBusMessage::createSignal(DBUS_PARH_DATA_MIGRATION, DBUS_INTERFACE_DATA_MIGRATION, "sigReceProcessFail");
            send << error;
            QDBusConnection::systemBus().send(send);
        });

        connect(m_processDataSize, &QProcess::readyReadStandardOutput, this, &DataMigration::processDataSizeOut);
    }
    m_processDataSize->close();
    QStringList options;
    switch (type)
    {
        //配置文件 etc
    case MigrateDataType::SysConfData:
        options << "-c"
                << " rsync -avzn --stats --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/etc/ " + m_receiverDataFile + "/";
        break;
        //用户 etc + home
    case MigrateDataType::UserData:
        options << "-c"
                << " rsync -avzn --stats --exclude-from="+ m_excludeHomeAndEtcFile +" --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
        //应用 etc + app
    case MigrateDataType::AppData:
        options << "-c"
                << " rsync -avzn --stats --exclude='home/*' --exclude-from="+ m_excludeFile +" --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
        //用户和配置文件 etc + home
    case MigrateDataType::UserAndSysData:
        options << "-c"
                << " rsync -avzn --stats --exclude-from="+ m_excludeHomeAndEtcFile +" --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
        //应用和配置文件 etc + app
    case MigrateDataType::AppAndSysData:
        options << "-c"
                << " rsync -avzn --stats --exclude='home/*' --exclude-from="+ m_excludeFile +" --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
        //应用和用户 etc + app +home
    case MigrateDataType::AppAndUserData:
        options << "-c"
                << " rsync -avzn --exclude-from="+ m_excludeFile +" --stats --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
    default:
        options << "-c"
                << "rsync -anvz --exclude-from="+ m_excludeFile +" --stats  --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/"; 
        break;
    }
    qDebug() << "===================" << options;
    m_processDataSize->start("bash", options);


}

void DataMigration::processDataSizeOut()
{   
    // 解析标准输出，获取总数据大小
    QString output = m_processDataSize->readAllStandardOutput();
    
    QDBusMessage send = QDBusMessage::createSignal(DBUS_PARH_DATA_MIGRATION, DBUS_INTERFACE_DATA_MIGRATION, "sigMigrateDataSize");
    QStringList lines = output.split("\n");
    foreach (const QString& line, lines) {
        if (line.startsWith("Total transferred file size:")) {

            // Total transferred file size: 433,954,396,528 bytes
            // line.split(":").at(1) ==> 433,954,396,528 bytes
            // sizeStr.split(" ")[0] ==> 433,954,396,528

            QString sizeStr = line.split(":").at(1).trimmed();
            
            QString numStr = sizeStr.split(" ")[0];

            QStringList numStrList = numStr.split(",");
            QString totalSizeStr = "";
            for (int i = 0; i < numStrList.size(); i++) {
                totalSizeStr = totalSizeStr + numStrList[i];
            }

            qint64 totalSize = totalSizeStr.toLongLong();
            qDebug() << "Total Data Size:" << totalSize;
            send << totalSize;
            QDBusConnection::systemBus().send(send);
            break;
        }
    }

}

void DataMigration::receiverMigrateData(int type)
{
    m_type = type;
    if (!m_processMigrateDate) {
        m_processMigrateDate = new QProcess(this);
        connect(m_processMigrateDate, &QProcess::readyReadStandardError, this,[=](){
            QString error = m_processMigrateDate->readAllStandardError();
            QDBusMessage send =
                    QDBusMessage::createSignal(DBUS_PARH_DATA_MIGRATION, DBUS_INTERFACE_DATA_MIGRATION, "sigReceProcessFail");
            send << error;
            QDBusConnection::systemBus().send(send);
        });
        connect(m_processMigrateDate, &QProcess::readyReadStandardOutput, this, &DataMigration::processMrigrationOut);
    }
    QString etcPath = m_receiverDataFile + "/etc/";
    QDir dirEtc = QDir(etcPath);
    if (!dirEtc.exists()) {
        dirEtc.mkpath(etcPath);
    }

    QStringList options;
    switch (type)
    {
    // 迁移etc目录
    case MigrateDataType::SysConfData:
        options << "-c"
                << " rsync -avz --info=progress2 --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/etc/ " + etcPath;
        break;
    // 迁移home和etc目录
    case MigrateDataType::UserData:
        options << "-c"
                << "rsync -avz --exclude-from="+ m_excludeHomeAndEtcFile +" --info=progress2  --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/"; 
        break;
    // 迁移etc 和 应用
    case MigrateDataType::AppData:
        options << "-c"
                << " rsync -avz --exclude='home/*' --exclude-from="+ m_excludeFile +" --info=progress2 --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
    // 迁移etc 和 应用
    case MigrateDataType::AppAndSysData:
        options << "-c"
                << " rsync -avz --exclude='home/*' --exclude-from="+ m_excludeFile +" --info=progress2 --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
    // 迁移home和etc目录
    case MigrateDataType::UserAndSysData:
        options << "-c"
                << " rsync -avz --exclude-from="+ m_excludeHomeAndEtcFile +" --info=progress2 --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
    // all
    case MigrateDataType::AppAndUserData:
        options << "-c"
                << " rsync -avz --exclude-from="+ m_excludeFile +" --info=progress2 --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
    case MigrateDataType::AllData:
        options << "-c"
                << " rsync -avz --exclude-from="+ m_excludeFile +" --info=progress2 --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
    default:
        options << "-c"
                << " rsync -avz --exclude-from="+ m_excludeFile +" --info=progress2 --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/ " + m_receiverDataFile + "/";
        break;
    }
    qDebug() << " -------------------------------- " << options;
    m_processMigrateDate->start("bash", options);

}

void DataMigration::processMrigrationOut()
{
    // 解析标准输出，获取文件名、当前速度和当前拉取数据大小
    QString output = m_processMigrateDate->readAllStandardOutput();
    QStringList lines = output.split("\n");
    foreach (const QString& line, lines) {
        QDBusMessage send = QDBusMessage::createSignal(DBUS_PARH_DATA_MIGRATION, DBUS_INTERFACE_DATA_MIGRATION, "sigMigrateDataInfo");
        if(line.startsWith("total size is")) {
            QDBusMessage sendSuccess = QDBusMessage::createSignal(DBUS_PARH_DATA_MIGRATION, DBUS_INTERFACE_DATA_MIGRATION, "sigMigrateDataSuccess");
            QDBusConnection::systemBus().send(sendSuccess);
        } else if (line.contains("created directory") || line.contains("./") || line.contains("receiving incremental file list") || line == "") {
            continue;
        } else if (line.contains("sent") && line.contains("received") && line.contains("bytes") && line.endsWith("bytes/sec")) {
            continue;
        } else if (line.startsWith("\r")) {
            QStringList fields = line.simplified().split(' ');
            QString fileName = ""; // 文件名
            QString speed = fields[2]; // 速度

            // QString dataReceived = fields[0];
            QStringList numStrList = fields[0].split(",");
            QString dataReceived = ""; // 大小
            for (int i = 0; i < numStrList.size(); i++) {
                dataReceived = dataReceived + numStrList[i];
            }

            QStringList list;
            list << fileName << dataReceived << speed;
            send << list;
            QDBusConnection::systemBus().send(send);
        } else {

            QString fileName = line;
            QString speed = "";
            QString dataReceived = "";
            QStringList list;
            list << fileName << dataReceived << speed;
            send << list;
            QDBusConnection::systemBus().send(send);
        }
    }  
}

void DataMigration::receiverReboot()
{
    QProcess::execute(QString("reboot"));
}

void DataMigration::receiverAppDataList(QStringList list)
{
    QFile file(m_appListFile);
    file.resize(0);
    file.close();
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        for(int i = 0; i < list.size() ;i++ ) {
            QString str = list[i] + "\n";
            QByteArray bytes = str.toUtf8();
            int len = file.write(bytes);
            if(len == 0) {
                qDebug() <<  " ===== DBUS === AppListFile write data failed !!!" << list[i];
            }
        }
        file.close();

    } else {
        qDebug() << " ===== DBUS === AppListFile failed to create!";
    }
    appDesktopToPkg(list);
}

void DataMigration::appDesktopToPkg(QStringList list)
{
    QStringList packageDesktopList;
    QStringList allDesktopsList;
    QHash<QString, QStringList> packDeskInfoHash;
    QHash<QString, QString> desktopPackageHash;
    QFile file(m_receiverDataFile + m_senderappInfoListFile);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qDebug()<<" Open json file failed ";
        return;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line == "") {
            continue;
        }
        // pack_v_all: desktop1 desktop2
        QStringList listInfo = line.split(":");
        QStringList desktopList = listInfo[1].split(" "); 
        foreach (const QString& listStr, desktopList) {
            desktopPackageHash.insert(listStr, listInfo[0]);
        }
        // packDeskInfoHash.insert(listInfo[0], desktopList);
    }

    // desktopPackageHash -> 存放的是 desktop (key) : package (value)
    // packDeskInfoHash -> 存放的是 package (key) : desktops (value)

    foreach (const QString& listStr, list) {
        if (desktopPackageHash.contains(listStr)) {
            QString valuePack = desktopPackageHash.value(listStr);
            // 判断 packDeskInfoHash 中key是否存在 valuePack ，存在将 listStr 存放在desktop文件列表中
            if (packDeskInfoHash.contains(valuePack)) {
                QStringList valueDesktopList = packDeskInfoHash.value(valuePack);
                valueDesktopList.append(listStr);
                packDeskInfoHash.insert(valuePack, valueDesktopList);
            } else {
                QStringList valueDesktopList;
                valueDesktopList.append(listStr);
                packDeskInfoHash.insert(valuePack, valueDesktopList);
            }
        }
    }

    QFile PackageInfofile(m_appInfoListFile);
    PackageInfofile.resize(0);
    PackageInfofile.close();
    if (PackageInfofile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)) {
        QHash <QString, QStringList>::const_iterator it;//迭代器的使用,it一开始指向的是第0个元素之前的位置
        for(it = packDeskInfoHash.constBegin(); it != packDeskInfoHash.constEnd(); ++it)
        {
            QString str = it.key() + ": " + it.value().join(" ") + "\n";
            QByteArray bytes = str.toUtf8();
            int len = PackageInfofile.write(bytes);
            if(len == 0) {
                qDebug() <<  " ===== DBUS === " << m_appInfoListFile << " write data failed !!!";
            }
        }
        PackageInfofile.close();
    } else {
        qDebug() << " ===== DBUS === " << m_appInfoListFile << " failed to create!";
    }
}

void DataMigration::receiverStopRsync()
{
    m_processMigrateDate->close();
}

void DataMigration::receiverConfigData()
{
    // 1.etc配置 2.home配置 3.应用配置
    qDebug() << m_type;
    switch (m_type)
    {
        // 2
    case MigrateDataType::UserData:
        configUseData();
        break;
        // 1
    case MigrateDataType::SysConfData:
        configSysData();
        break;
        // 3
    case MigrateDataType::AppData:
        configAppData();
        break;
        // 1+3
    case MigrateDataType::AppAndSysData:
        configSysData();
        configAppData();
        break;
        // 1+2
    case MigrateDataType::AppAndUserData:
        configUseData();
        configAppData();
        break;
        // 1+2+3
    case MigrateDataType::UserAndSysData:
        configSysData();
        configUseData();
        break;
        // 1+2+3
    case MigrateDataType::AllData:
        configSysData();
        configUseData();
        configAppData();
        break;    
    default:
        break;
    }
        // configAppData();
}

void DataMigration::receiverMigrateVersionFile()
{
    QProcess process;

    QStringList options;
    options << "-c"
            << " rsync -avz --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile" + m_senderDataDir + " " + m_senderDataDir;
    process.start("bash", options);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
            
    process.close();

    // 判断文件是否存在，存在生成文件夹名称
    QFileInfo fileInfo(m_senderVersion);
    if (!fileInfo.isFile()) {
        m_receiverFilePathName = "fs.old-v10sp1";
        m_receiverDataFile = "/"  + m_receiverFilePathName;
        receiverMigrateAppData();
        return;
    }
    QFile file(m_senderVersion);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file!";
        m_receiverFilePathName = "fs.old-v10sp1";
        m_receiverDataFile = "/"  + m_receiverFilePathName;
        receiverMigrateAppData();
        return;
    }
    QString info = file.readAll().trimmed();
    if (info.startsWith("ubuntu16.04", Qt::CaseInsensitive)) {
        m_receiverFilePathName = "fs.old-u16";
    } else if (info.startsWith("ubuntu18.04", Qt::CaseInsensitive)) {
        m_receiverFilePathName = "fs.old-u18";
    } else if (info.startsWith("ubuntu20.04", Qt::CaseInsensitive)) {
        m_receiverFilePathName = "fs.old-u20";
    } else if (info.startsWith("ubuntu22.04", Qt::CaseInsensitive)) {
        m_receiverFilePathName = "fs.old-u22";
    } else if (info.startsWith("openkylin1.0", Qt::CaseInsensitive)) {
        m_receiverFilePathName = "fs.old-ok1";
    } else if (info.startsWith("openkylin2.0", Qt::CaseInsensitive)) {
        m_receiverFilePathName = "fs.old-ok2";
    } else if (info.startsWith("v4", Qt::CaseInsensitive)) {
        m_receiverFilePathName = "fs.old-v4";
    } else if (info.startsWith("v10", Qt::CaseInsensitive)) {
        m_receiverFilePathName = "fs.old-v10";
    } else if (info.contains("v10sp1", Qt::CaseInsensitive)) {
        m_receiverFilePathName = "fs.old-v10sp1";
    } else {
        m_receiverFilePathName = "fs.old-v10sp1";
    }
    m_receiverDataFile = "/"  + m_receiverFilePathName;

    receiverMigrateAppData();
}

void DataMigration::receiverMigrateAppData()
{
    QString path = "usr/share/applications/";
    QString getDataPath = m_receiverDataFile + "/" + path;

    QDir confDir = QDir(getDataPath);
    if (!confDir.exists()) {
        confDir.mkpath(getDataPath);
    }
    if (m_processMigrateDektopDate == nullptr ) {
        m_processMigrateDektopDate = new QProcess();
        connect(m_processMigrateDektopDate, &QProcess::readyReadStandardOutput, this, [=](){
            // 解析标准输出，获取文件名、当前速度和当前拉取数据大小
            QString output = m_processMigrateDektopDate->readAllStandardOutput();
            
            QStringList lines = output.split("\n");
            foreach (const QString& line, lines) {
                if(line.startsWith("total size is")) {
                    QDBusMessage send = QDBusMessage::createSignal(DBUS_PARH_DATA_MIGRATION, DBUS_INTERFACE_DATA_MIGRATION, "sigMigrateAppDataSuccess");
                    QDBusConnection::systemBus().send(send);
                } 
            }
        });
    }
    QStringList options;
    options << "-c"
            << " rsync -avz --password-file=" + m_recePwdFile + " " + m_name + "@" + m_ip + "::shareFile/" + path + " " + getDataPath;
    m_processMigrateDektopDate->start("bash", options);

    m_processMigrateDektopDate->waitForFinished();
}

void DataMigration::senderGetAppDesktopInfo()
{
    // 读取/usr/share/application/的所有desktop文件
    QDir* dirinfo = new QDir(desktopFilePath);
    if (!dirinfo->exists()) {
        delete dirinfo, dirinfo = nullptr;
    }
    dirinfo->setNameFilters(QStringList("*"));
    QStringList fileList = dirinfo->entryList(QDir::Files);

    fileList.removeOne(".");
    fileList.removeOne("..");
    QStringList desktopList;
    desktopList.clear();
    QString desktopStr;

    for (int i = 0; i < fileList.size(); i++ ) {

        QString file = desktopFilePath + fileList[i];

        QSettings desktopFileSetting(file, QSettings::IniFormat);
        desktopFileSetting.setIniCodec(QTextCodec::codecForName("UTF-8"));
        desktopFileSetting.beginGroup(DesktopGroup);
        QString name = desktopFileSetting.value(DesktopKeyNameZH).toString();
        QString comm = desktopFileSetting.value(DesktopKeyCommentZH).toString();
        QString Exec = desktopFileSetting.value("Exec").toString();
        if (name == "") {
            name = desktopFileSetting.value(DesktopKeyNameEN).toString();
        } 
        if (comm == "") {
            comm = desktopFileSetting.value(DesktopKeyCommentEN).toString();
        }
        if (name == "") {
            continue;
        }
        desktopList.append(desktopFilePath + fileList[i]);
        desktopStr = desktopStr + " " + desktopFilePath + fileList[i];
    }
    qDebug() << " sender /usr/share/application/的desktop文件数量: " << desktopList.size();

    if ( m_processGetDesktopInfo == nullptr) {
        m_processGetDesktopInfo = new QProcess();
    }
    // 清空文件和正确的desktop的列表，应用包名列表
    QFile file(m_senderappInfoListFile);
    file.resize(0);
    QFile desktopFile(m_senderappListFile);
    desktopFile.resize(0);
    m_desktopFileIsRightList.clear();
    m_packsgeNameList.clear();
    m_hashPackage.clear();

    m_processGetDesktopInfo->start("dpkg -S " + desktopStr);
    m_processGetDesktopInfo->waitForFinished();

    processDesktopToPackageOut();
    // 使用dpkg -l获取包信息
    senderGetAppPackageInfo(m_packsgeNameList);

    // 创建文件夹 /tmp/komt/
    QDir dataDir = QDir(m_senderDataDir);
    if (!dataDir.exists()) {
        dataDir.mkpath(m_senderDataDir);
    }

    senderGetVersion();
    senderGetUserInfo();
}

void DataMigration::processDesktopToPackageOut()
{
    QString outPut = m_processGetDesktopInfo->readAllStandardOutput();
    QStringList lines = outPut.split("\n");

    foreach (const QString& line, lines) {
        if (line == "") {
            continue;
        }
        QStringList packageList = line.split(":");
        QString packageName = packageList[0];
        QStringList desktopFileList = packageList[1].split("/");
        QString desktopFile = desktopFileList.last();

        // 将数据 应用包名，desktop文件写到哈希表中，将正确的desktop文件名字写到，应用包列表
        if (m_hashPackage.contains(packageName)) {
            QStringList valueList = m_hashPackage.value(packageName);
            valueList.append(desktopFile);
            m_hashPackage[packageName] = valueList;
        } else {
            QStringList valueList;
            valueList.append(desktopFile);
            m_hashPackage.insert(packageName, valueList);
            m_packsgeNameList.append(packageName);
        }
        m_desktopFileIsRightList.append(desktopFile);
        
    }

    // 将正确的desktop文件列表数据写入文件中
    QFile desktopFile(m_senderappListFile);
    desktopFile.close();
    if (desktopFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        for(int i = 0; i < m_desktopFileIsRightList.size() ;i++ ) {
            QString str = m_desktopFileIsRightList[i] + "\n";
            QByteArray bytes = str.toUtf8();
            int len = desktopFile.write(bytes);
            if(len == 0) {
                qDebug() <<  " ===== DBUS === " << m_senderappListFile << " write data failed !!!" << m_desktopFileIsRightList[i];
            }
        }
        desktopFile.close();

    } else {
        qDebug() << " ===== DBUS === " << m_senderappListFile << " failed to create!";
    }
}

void DataMigration::senderGetAppPackageInfo(QStringList list)
{
    if ( m_processGetPackageInfo == nullptr) {
        m_processGetPackageInfo = new QProcess();
        m_desktopFileIsRightList.clear();

        // 设置要执行的命令和参数
        QStringList args;
        args << "-l";
        args << list;

        m_processGetPackageInfo->setProgram("dpkg");
        m_processGetPackageInfo->setArguments(args);
    }
    QFile file(m_senderappInfoListFile);
    file.resize(0);

    m_processGetPackageInfo->start();
    if (!m_processGetPackageInfo->waitForFinished()) {
        qDebug() << "Failed to start dpkg process.";
    }
    processPackageInfoOut();
}

void DataMigration::processPackageInfoOut()
{
    QString outPut = m_processGetPackageInfo->readAllStandardOutput();
    QStringList lines = outPut.split("\n");

    for (const QString& line : lines) {
        if (!line.contains("ii")) {
            continue;
        }
        QStringList parts = line.split(" ", QString::SkipEmptyParts);
        
        if (parts.length() < 3) {
            continue;
        }
        QString packageName = parts[1];
        QString packageVersion = parts[2];
        QString packageType = parts[3];
        QString hashKey = packageName + "_" + packageVersion + "_" + packageType;
        QStringList list = m_hashPackage.value(packageName);
    
        QFile file(m_senderappInfoListFile);
        file.close();
        if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append)) {
            QString str = hashKey + ": " + list.join(" ") + "\n";
            QByteArray bytes = str.toUtf8();
            int len = file.write(bytes);
            if(len == 0) {
                qDebug() <<  " ===== DBUS === " << m_senderappInfoListFile << " write data failed !!!";
            }
            file.close();
        } else {
            qDebug() << " ===== DBUS === " << m_senderappInfoListFile << " failed to create!";
        }
    }
}

QStringList DataMigration::readYamlFile(QString str)
{
    QStringList list;
    list.clear();
    QFile filePath(m_configDataYamlFile);
    if (!filePath.open(QIODevice::ReadOnly)) {
        return list;
    }
    QString rules = filePath.readAll();
    QStringList tmpRuls = rules.split("\n");
    bool shouldRuleFiles = false;
    for (const QString &rule: tmpRuls) {
        if (!rule.startsWith(" ")) {
            shouldRuleFiles = false;
            if (rule == str) {
                shouldRuleFiles = true;
            }
        }
        if (shouldRuleFiles) {
            QString tmp = rule;
            tmp = tmp.split(" ").last();
            list.append(tmp);
        }
    }
    list.removeAll(str);
    return list;
}

void DataMigration::configSysData()
{
    qDebug() << "++ configSysData ++";
    m_listFile.clear();
    m_listFolders.clear();

    m_listFile = readYamlFile("movefile:");
    qDebug() << m_listFile;
    m_listFolders = readYamlFile("movefolder:");
    qDebug() << m_listFolders;

    for (int i = 0; i < m_listFile.size(); i++) {
        QProcess process;
        QStringList options;
        options << m_receiverDataFile + "/etc/" + m_listFile[i] << "/etc";
        process.start("cp", options);
        process.waitForFinished();
        qDebug() << " ==== etc文件cp ==== " ;
    }
    for (int i = 0; i < m_listFolders.size(); i++) {
        QProcess process;
        QStringList options;
        options << "-r" << m_receiverDataFile + "/etc/" + m_listFolders[i] << "/etc";
        process.start("cp", options);
        process.waitForFinished();
        qDebug() << " ==== etc文件夹cp ==== " ;
    }

}

QStringList DataMigration::getHomeDirChildDir(QString str)
{
    // 读取当前home目录下存在的用户名
    QStringList currentUserNameList;
    currentUserNameList.clear();

    QString homePath = str;
    QDir directory(homePath);

    // 获取目录 /home 中的所有子目录
    QStringList allDirectories = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 遍历子目录，将目录名存入 QStringList
    foreach (const QString& dirName, allDirectories) {
        QString subDirectoryPath = homePath + "/" + dirName;
        QDir subDirectory(subDirectoryPath);
        currentUserNameList << dirName;
    }
    return currentUserNameList;
}

QStringList DataMigration::getUserInfo(QString uname, QString filePath)
{
    QStringList listInfo;
    listInfo.clear();
    // 打开文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file!" << filePath;
        return listInfo;
    }

    // 读取文件内容并判断每一行的开头
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();

        // 判断开头是否包含指定字符串
        listInfo = line.simplified().split(':');
        if (listInfo[0] == uname) {
            break;
        }
        listInfo.clear();
    }

    // 关闭文件
    file.close();
    return listInfo;

}

bool DataMigration::NewSysExistUid(int id)
{
    QString etcNewPath ="/etc/";
    QString uidlistFile = "uid_list";

    QFileInfo oldUidListFile = QFileInfo(etcNewPath + uidlistFile);
    if (!oldUidListFile.isFile()) {
        uidlistFile = "group";
    }

    // 打开文件
    QFile file(etcNewPath + uidlistFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file!";
        return true;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList strList = line.simplified().split(':');
        if (strList.contains(QString::number(id))) {
            return true;
        } 
    }
    return false;
}

void DataMigration::updateNewSysEtcFile(QString filePath, QStringList list, int oldUid, int newUid)
{
    qDebug() << filePath << "list ==== " << list << " ===== " << oldUid << newUid;

    for (int i = 0; i < list.size(); i++ ) {
        QString s = list[i];
        if (s == QString::number(oldUid)) {
            list[i] = QString::number(newUid);
        }
    }
    // QString str = list.join(":");
    qDebug() << filePath << "list ==== " << list << " ===== " << oldUid << newUid;


    // 打开文件以进行追加写入
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QTextStream out(&file);
        // foreach遍历
        for (int i = 0; i < list.size(); i++ ) {
            QString s = list[i];
            if (s == QString::number(oldUid)) {
                list[i] = QString::number(newUid);
            }
        }
        QString str = list.join(":");
        // 将内容写入文件末尾
        out << str << "\n";
        // 关闭文件
        file.close();
    } else {
        qDebug() << "Failed to open file for appending!";
    }

}

void DataMigration::configUseData()
{
    /*
     * 1. 获取当前系统中存在的用户（home下的子目录名字）
     * 2. 获取老系统中存在的用户（fs.old的home下的子目录）
     * 3. 获取需要在当前/etc目录下修改的文件
     * 4. 进行循环判断当前系统老系统用户名是否相同，不相同进行后续处理
     * 5. 获取老系统的/etc/uid_list，获取到对应用户名的UID；
     * 6. 将老系统用户的uid，与当前uidlist进行比对，若自增id；
     * 7. 将用户名，uid更改所有当前/etc目录下修改的文件
    */
qDebug() << "++ configUseData ++";
    // 读取home目录下存在的用户名【步骤1，2】
    QStringList currentUserNameList = getHomeDirChildDir("/home");
    QStringList oldUserNameList = getHomeDirChildDir(m_receiverDataFile + "/" + "home");

    // 获取需要在当前/etc目录下修改的文件【步骤3】
    QStringList list = readYamlFile("userfile:");

    QString etcNewPath ="/etc/";
    QString etcOldPath = m_receiverDataFile + "/" + "etc/";
    QString uidlistFile = "uid_list";
    QString getUserFilePath;

    int uidNum = 1100;
    QStringList info;

    qDebug() << " currentUserNameList " << currentUserNameList;
    qDebug() << " oldUserNameList " << oldUserNameList;

    for (const auto &i: oldUserNameList) {
        // 判断新机器的用户列表包含老机器的用户列表
        if ( currentUserNameList.contains(i)) {
            continue;
        }
        // 判断文件/fs.old/etc/uid_list是否存在
        QFileInfo oldUidListFile = QFileInfo(etcOldPath + uidlistFile);
        if (!oldUidListFile.isFile()) {
            getUserFilePath = m_senderUserInfo;
        } else {
            getUserFilePath = etcOldPath + uidlistFile;
        }
        qDebug() << " getUserFilePath " << getUserFilePath;
        // 使用老用户名，获取信息
        info = getUserInfo(i, getUserFilePath);
        qDebug() << " info " << info;
        int oldUid = info.at(1).toInt();
        while (NewSysExistUid(uidNum)) {
            uidNum++;
            qDebug() << uidNum;
        }
        // 处理etc目录下的对应文件
        for(int k = 0; k < list.size(); k++ ) {
            QStringList userInfo = getUserInfo(i, etcOldPath + list[k]);
            qDebug() << userInfo;
            if (userInfo.isEmpty()) {
                continue;
            }
            updateNewSysEtcFile(etcNewPath + list[k], userInfo, oldUid, uidNum);
        }
    }

    QProcess process;
    QStringList options;
    options << "-c" << "cp -rn " + m_receiverDataFile + "/home/* /home/";
    qDebug() << options;
    process.start("bash", options);
    process.waitForFinished();

}

void DataMigration::configAppData()
{
    // 读取被选中的desktop列表中的每个desktop文件的内容
    QFile file(m_appListFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file!";
        return;
    }

    // 读取文件内容并判断每一行的开头
    QTextStream in(&file);
    // 拷贝图片
    QString filePath = m_receiverDataFile + "/usr/share/applications/";
    QString iconPath = "/usr/share/icons/";
    QString DesktopGroup = "Desktop Entry";
    while (!in.atEnd()) {
        QString line = in.readLine();
        QSettings desktopFileSetting(filePath + line, QSettings::IniFormat);
        // QSettings desktopFileSetting(iconPath + line, QSettings::IniFormat);
        desktopFileSetting.setIniCodec(QTextCodec::codecForName("UTF-8"));
        desktopFileSetting.beginGroup(DesktopGroup);
        QString Icon = desktopFileSetting.value("Icon").toString();
        if (Icon == "") {
            continue;
        } 
        if (Icon.startsWith("/")) {
            QProcess process;
            QStringList option;
            option << "-n" << m_receiverDataFile + Icon << Icon;
            process.start("/usr/bin/cp", option);
            process.waitForFinished();
        } else {
            QProcess process;
            QStringList option;
            option << m_receiverDataFile + iconPath << "-name" << "\""+ Icon +"*\"";
            process.start("/usr/bin/find", option);
            process.waitForFinished();
            // 解析标准输出，提取带有 "h" 的文件或目录
            QString output = process.readAllStandardOutput();
            QStringList lines = output.split("\n");
            
            // 遍历子目录，将目录名存入 QStringList
            foreach (const QString& line, lines) {
                QStringList list = line.split("/");
                QStringList newList;
                newList.clear();
                for(int i = 0; i< list.size(); i++) {
                    if(list[i] != m_receiverFilePathName) {
                        newList.append(list[i]);
                    }
                }
                QString newIcon = newList.join("/");
                QProcess p;
                QStringList opt; 
                opt<< "-c" << "cp -n " + line + " " + newIcon;
                p.start("bash", opt);
                p.waitForFinished();
            }
        } 
    }
    startKare();
}

// QStringList DataMigration::findIcon(QString path, QString name)
// {
//     QStringList list;
//     list.clear();
//     QDir directory(path);
//     QStringList allEntries = directory.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

//     // 遍历文件和子目录，查找目标文件 b
//     foreach (const QString& entry, allEntries) {
//         QString entryPath = path + "/" + entry;
//         QFileInfo fileInfo(entryPath);

//         // 如果找到目标文件 b，输出其路径
//         if (fileInfo.fileName() == name && fileInfo.isFile()) {
//             qDebug() << "Found file" << fileInfo.filePath();
//             list << entryPath;
//         } else if (fileInfo.isDir()) {
//             list = findIcon(entryPath, name);
//         }
//     }
//     return list;
// }

void DataMigration::startKare()
{
    QProcess process;
    QStringList opt;
    opt << "-c" << "/usr/bin/buildCompatibilityEnv.sh";
    process.start("bash", opt);
    process.waitForStarted();
    process.waitForFinished();

}
