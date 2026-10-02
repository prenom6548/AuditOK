#include "diskinfofinder.h"

#include <QProcess>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QRegularExpression>
#include <QProcess>
#include <QStringList>
#include <QByteArray>

DiskInfoFinder::DiskInfoFinder() {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &DiskInfoFinder::dowork);

    connect(this, &DiskInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &DiskInfoFinder::finished, this, &DiskInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

DiskInfoFinder::~DiskInfoFinder() {

}

void DiskInfoFinder::dowork()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);
    if(manager.isValid()) {
        QDBusMessage response = manager.call("readHDInfo");

        QString result = response.arguments().at(0).toString();
        qDebug() << result;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());

        if (jsonDoc.isObject()) {
            QJsonObject jsonObject = jsonDoc.object();
            if (jsonObject.contains("list") && jsonObject["list"].isArray()) {
                QJsonArray jsonArray = jsonObject["list"].toArray();

                // 遍历数组
                for (const QJsonValue &value: jsonArray) {
                    if (value.isObject()) {
                        QJsonObject diskObject = value.toObject();

                        DiskInfo diskinfo;
                        diskinfo.name = diskObject.value("name").toString();
                        diskinfo.size = diskObject.value("capacity").toString();
                        diskinfo.model = diskObject.value("model").toString();
                        diskinfo.vendor = diskObject.value("manufacturer").toString();
                        diskinfo.serial = diskObject.value("serial_num").toString();
                        diskinfo.frimwareversion = diskObject.value("firmware_ver").toString();
                        diskinfo.is_main = diskObject.value("is_maindisk").toString().toInt();
                        diskinfo.interface = diskObject.value("interface").toString();
                        diskinfo.type = getDiskType(diskinfo.name);
                        diskinfo.ufs = diskObject.value("disk_ufs").toString();
                        diskinfo.ufsrevision = diskObject.value("ufs_revision").toString(); // 将解析出来的 keyboardInfo 添加到 QList
                        diskinfo.rota = diskObject.value("rota").toString();
                        if (diskinfo.ufs == ""){
                            diskinfo.ufs = getUFSInfo(diskinfo.name);
                        }
                        // 将解析出来的 keyboardInfo 添加到 QList
                        m_diskInfo.append(diskinfo);
                    }
                }
            }
        }
    }
    emit succeed();
}



QList<DiskInfo> DiskInfoFinder::getDiskInfo(){
    return m_diskInfo;
}

QString DiskInfoFinder::getUFSInfo(QString name) {
    QString dev = QString(name.split("/").last()).trimmed();
    QString path = "/sys/block/" + dev + "/device/spec_version";

    // 打开文件
    QFile file(path);
    if (!file.exists()) {
        return QString();
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    // 读取文件内容
    QTextStream in(&file);
    QString specVersion = in.readAll().trimmed();
    file.close();
    if (specVersion.toInt() > 300){
        return "3.1";
    }else if (specVersion.toInt() == 300){
        return "3.0";
    }else{
        return "2.0";
    }
}

QString DiskInfoFinder::getDiskType(QString name) {
    // 0 HDD   1 SSD   2 U盘类移动盘  3 hwssd显示ufs
    QString name1 = name.split("/").last();
    QProcess proc;
    QString cmd = "lsblk -d -o NAME,TRAN,ROTA,HOTPLUG";
    proc.start(cmd);
    proc.waitForFinished();
    QStringList outPut = QString(proc.readAllStandardOutput()).split("\n", QString::SkipEmptyParts);
    for (int i = 1; i < outPut.length(); ++i) {
        if (outPut[i].contains(name1)){
            if (QString(QString(QString(outPut[i]).remove(name1).trimmed()).split(" ", QString::SkipEmptyParts)[1]).toInt() == 1 && outPut[i].contains("sata")){
                return "0";
            }
            if(QString(outPut[i]).split(" ", QString::SkipEmptyParts).last().toInt() == 1 && (!outPut[i].contains("sata"))){
                return "2";
            }
            if (QString(QString(QString(outPut[i]).remove(name1).trimmed()).split(" ", QString::SkipEmptyParts)[1]).toInt() == 0){
                // 先判断是不是华为设备，如果是华为设备则返回3
                if(judge990() || judge9006C() || judgePANGUV() || judgePANGUX()){
                    return "3";
                }
                return "1";
            }
        }
    }
    proc.close();

    // 默认返回 0
    qDebug() << "Disk type not obtained, using default type: HDD";
    return "0";
}

bool DiskInfoFinder::judge990()
{
    QFile file("/proc/hardware");
    if (!file.exists()) {
        return false;
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString info = in.readLine().toLower();
        file.close();

        QRegularExpression regex("kirin.*990");
        if (regex.match(info).hasMatch() || info.contains("huawei kirin 990") || info.contains("kirin990") || info.contains("huawei kirin 9006c")) {
            return true;
        }
    }
    return false;
}

bool DiskInfoFinder::judge9006C()
{
    QFile file("/proc/hardware");
    if (!file.exists()) {
        return false;
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString info = in.readLine().toLower();
        file.close();

        QRegularExpression regex("kirin.*9006c");
        if (regex.match(info).hasMatch() || info.contains("huawei kirin 9006c")) {
            return true;
        }
    }
    return false;
}

bool DiskInfoFinder::judgePANGUX()
{
    QProcess process;
    QStringList args;
    args << "--oem-string" << "4";
    process.start("dmidecode", args);
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QList<QByteArray> lines = output.split('\n');

    for (const QByteArray& line : lines) {
        QString decodedLine = QString::fromUtf8(line.trimmed());
        if (decodedLine == "PGUX") {
            // 获取 dmidecode -t baseboard 的值并检查
            QProcess baseboardProcess;
            baseboardProcess.start("dmidecode", QStringList() << "-t" << "baseboard");
            baseboardProcess.waitForFinished();
            QByteArray baseboardOutput = baseboardProcess.readAllStandardOutput();
            baseboardProcess.close();

            QString baseboardInfo = QString::fromUtf8(baseboardOutput);
            if (baseboardInfo.contains("SP1PANGUXM")) {
                return true;
            }
            return false;
        }
    }
    return false;

}

bool DiskInfoFinder::judgePANGUV()
{
    QProcess process;
    QStringList args;
    args << "--oem-string" << "4";
    process.start("dmidecode", args);
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QList<QByteArray> lines = output.split('\n');

    for (const QByteArray& line : lines) {
        QString decodedLine = QString::fromUtf8(line.trimmed());
        if (decodedLine.contains("PGK")) {
            // 获取 dmidecode -t baseboard 的值并检查
            QProcess baseboardProcess;
            baseboardProcess.start("dmidecode", QStringList() << "-t" << "baseboard");
            baseboardProcess.waitForFinished();
            QByteArray baseboardOutput = baseboardProcess.readAllStandardOutput();
            QString baseboardInfo = QString::fromUtf8(baseboardOutput);

            if (baseboardInfo.contains("SP1PGUVMM")) {
                return true;
            }
            return false;
        }
    }
    return false;
}


