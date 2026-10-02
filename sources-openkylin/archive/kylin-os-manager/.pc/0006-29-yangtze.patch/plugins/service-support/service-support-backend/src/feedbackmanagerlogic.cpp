#include "feedbackmanagerlogic.h"
#include <QDebug>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QDBusInterface>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkReply>
#include <QEventLoop>
#include <QHttpPart>
#include <QThread>
#include <QMessageAuthenticationCode>
#include "settings.h"
#include "informationitem.h"
#include "dbus_args.h"

#define SINGLE_LOGFILE_SIZE_MAX 1024 * 1024 * 60

FeedbackManagerLogic::FeedbackManagerLogic(const ModuleTypeSet &set, InformationUserStruct userData,
                                           InformationClassItemList allItems, const QString &savePath, bool *cancel,
                                           bool isRetry)
    : m_typeSet(set), m_savePath(savePath), m_userData(userData), m_allItems(allItems), m_cancel(cancel),
      m_isRetry(isRetry)
{}

FeedbackManagerLogic::~FeedbackManagerLogic()
{
    m_proc->deleteLater();
    m_dbus->deleteLater();
    m_networkManager->deleteLater();
}

void FeedbackManagerLogic::startCollect()
{
    emit creatProgress(0);
    m_networkManager = new QNetworkAccessManager;
    if (m_isRetry) {
        uploadData();
        return;
    }
    if (m_time == nullptr) {
        m_time = new QTime;
        m_time->start();
    } else {
        m_time->restart();
    }
    m_proc = new QProcess();
    connect(m_proc, &QProcess::readyRead, this, &FeedbackManagerLogic::getProgress);
    connect(m_proc, SIGNAL(finished(int)), this, SLOT(onProcFinish(int)));
    m_dbus = new QDBusInterface(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_TOOL, dbus::DBUS_INTERFACE_TOOL,
                                QDBusConnection::systemBus());
    //创建临时目录
    m_timeStr = QString::number(QDateTime::currentDateTime().toSecsSinceEpoch());
    QString hostName = qgetenv("USER");
    QString tmpPath = "/tmp/kom-pfb." + hostName + "_" + m_timeStr + "/";
    m_tmpPath = tmpPath + "kylin-os-manager";
    if (!QDir().mkpath(m_tmpPath)) {
        qDebug() << "creat tmp path error";
        emit errorMessage(tr("Failed to create temporary directory!"));
        return;
    };
    m_savePath = m_savePath + "/" + QString("附件_") + hostName + "." + m_timeStr + ".7z";
    //缓存用户数据
    saveUserData();
    qDebug() << "缓存用户数据完成：" << m_time->elapsed();
    //采集信息
    collecting();
    qDebug() << "采集系统信息完成：" << m_time->elapsed();
    if (*m_cancel) {
        finish(Cancel);
        return;
    }
    //创建压缩包
    creatPackage();
}


void FeedbackManagerLogic::collecting()
{
    int classItemNumber = m_allItems.length();
    for (int i = 0; i < classItemNumber; i++) {
        InformationClassItem *cit = m_allItems[i];
        for (InformationItem *it : *cit->children()) {
            if (*m_cancel) {
                return;
            }
            if (!cit->isSelect() && !it->hasType(m_typeSet)) {
                continue;
            }
            QString tmpSaveDir = m_tmpPath + "/" + cit->getItemName() + "/" + it->getItemName() + "/";
            if (!QDir().mkpath(tmpSaveDir)) {
                qDebug() << "creat tmp dir error" << tmpSaveDir;
                continue;
            };
            switch (it->collectionType()) {
            case InformationItem::File:
                collectingFile(tmpSaveDir, it->getMessage(), cit->Detailed());
                break;
            case InformationItem::CMD:
                collectingCmd(tmpSaveDir, it->getMessage());
                break;
            case InformationItem::Others:
                collectingOther(tmpSaveDir, it->getMessage(), it->getItemName());
                break;
            default:
                break;
            }
        }
        emit creatProgress((i + 1) * 25 / classItemNumber);
    }
}

void FeedbackManagerLogic::creatPackage()
{
    m_proc->waitForReadyRead();
    QString cmd = "/usr/lib/p7zip/7za";
    QStringList args;
    args.append("a");
    args.append("-y");
    args.append("-l");
    args.append("-bsp1");
    args.append("-m0=lzma2");
    args.append("-mx=9");
    args.append(m_savePath);
    args.append(m_tmpPath);
#ifdef DEBUG_MODE
    QString tmpCmd = cmd;
    for (const QString &str : args) {
        tmpCmd += " " + str;
    }
    qDebug() << tmpCmd;
#endif
    m_proc->start(cmd, args);
}

void FeedbackManagerLogic::getProgress()
{
    if (*m_cancel) {
        return;
    }
    QString out = m_proc->readAllStandardOutput();
    QStringList tmp = out.split(' ');
    tmp.removeAll("");
    for (const QString &str : tmp) {
        if (str.contains('%')) {
            QString value = str;
            value.remove('%');
            bool ok = false;
            int v = value.toInt(&ok);
            if (ok) {
                emit creatProgress(25 + v / 4);
            }
        }
    }
}


void FeedbackManagerLogic::saveUserData()
{
    QString userPath = m_tmpPath + "/" + "用户提交";
    if (!QDir().mkpath(userPath)) {
        qDebug() << "creat tmp dir error" << userPath;
        return;
    };
    QFile file(userPath + "/提交内容.txt");
    if (!file.open(QIODevice::NewOnly)) {
        qDebug() << "creat user data file error";
        return;
    }
    QByteArray info;
    //    info.append("【类别】\n" + QString::number(m_userData.type) + "\n");
    info.append("【标题】\n" + m_userData.title + "\n");
    info.append("【内容】\n" + m_userData.originalDetails + "\n");
    info.append("【手机号】\n" + m_userData.phoneNumber + "\n");
    info.append("【邮箱】\n" + m_userData.mail + "\n");
    info.append("【称谓】\n" + m_userData.name + "\n");
    file.write(info);
    file.close();
    for (const QString &f : m_userData.accessory) {
        QString path = "\"" + f + "\"";
        QString cmd = "cp " + path + " " + userPath;
        system(cmd.toLocal8Bit().data());
    }
}

void FeedbackManagerLogic::uploadData()
{
    /* 读取 url 信息 */
    auto [protocol, domain, port] = Settings::getUrlInformation();
    QString baseUrl = QString("%1://%2").arg(protocol).arg(domain);
    if (!port.isEmpty()) {
        baseUrl += ":" + port;
    }
    baseUrl += "/creatbug";

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    appendHttpPart(multiPart, "title", m_userData.title);
    appendHttpPart(multiPart, "usermail", m_userData.mail);
    appendHttpPart(multiPart, "classtype", m_userData.priviteType);
    appendHttpPart(multiPart, "steps", m_userData.details);
    if (m_userData.giteecode.isEmpty()) {
        m_userData.giteecode = "0";
    }
    appendHttpPart(multiPart, "giteecode", m_userData.giteecode);
    appendHttpPart(multiPart, "files", m_savePath);

    QNetworkRequest creatReq;
    creatReq.setUrl(QUrl(baseUrl));
    QSslConfiguration tmpSSL = creatReq.sslConfiguration();
    tmpSSL.setPeerVerifyMode(QSslSocket::VerifyNone);
    creatReq.setSslConfiguration(tmpSSL);
    m_networkReply = m_networkManager->post(creatReq, multiPart);
    connect(m_networkReply, &QNetworkReply::uploadProgress, this, &FeedbackManagerLogic::uploadProgress);
    connect(m_networkReply, &QNetworkReply::finished, this, &FeedbackManagerLogic::uploadFinish);
    multiPart->setParent(m_networkReply);
}

void FeedbackManagerLogic::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    if (bytesTotal < 1) {
        return;
    }
    emit creatProgress(50 + bytesSent * 50 / bytesTotal - 1);
}

void FeedbackManagerLogic::uploadFinish()
{
    QByteArray creatByt = m_networkReply->readAll();
    m_networkReply->deleteLater();
    qDebug() << "uploadFinish :" << creatByt;
    QVariant idVar = QJsonDocument::fromJson(creatByt).object().value("bugid");
    QString tmp = idVar.toString().split(":").last();
    if (!tmp.isEmpty() && tmp != "0") {
        Settings::setHistoryBug(idVar.toString());
        finish(Success);
    } else if (idVar.toInt() > 0) {
        Settings::setHistoryBug(QString::number(m_userData.module) + ":" + QString::number(idVar.toInt()));
        finish(Success);
    } else {
        finish(UploadFail, m_savePath);
    }
}

void FeedbackManagerLogic::cancel()
{
    if (m_networkReply) {
        m_networkReply->abort();
        m_networkReply->deleteLater();
    }
    finish(Cancel);
}

void FeedbackManagerLogic::finish(FeedBackFinishType type, QString str)
{
    if (type == Cancel && m_cancelSuccess) {
        return;
    }
    if (!m_isRetry) {
        qDebug() << "采集结束，本次总用时：" << m_time->elapsed();
    }
    emit creatFinish(type, str);
}

void FeedbackManagerLogic::appendHttpPart(QHttpMultiPart *multiPart, const QString &name, const QString &value)
{
    if (value.isEmpty()) {
        return;
    }
    QHttpPart part;
    if (name == "files") {
        QFile *fileIO = new QFile(value);
        fileIO->setParent(multiPart);
        bool ok = fileIO->open(QIODevice::ReadOnly);
        QString filename = QFileInfo(value).fileName();
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data;name=\"" + name + "\";filename=\"" + filename + "\";"));
        m_posData.append(filename.toLocal8Bit());
        m_posData.append(fileIO->readAll());
        fileIO->reset();
        part.setBodyDevice(fileIO);
        qDebug() << "待上传文件状态：" << value << ok;
    } else {
        part.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data;name=\"" + name + "\"");
        part.setBody(value.toLocal8Bit());
    }

    multiPart->append(part);
}

void FeedbackManagerLogic::collectingFile(const QString &saveDir, const QString &message, int detailed)
{
    int tstart = m_time->elapsed();
    if (detailed < 1) {
        detailed = 1;
    }
    QString file = message;
    QStringList list;
    QDBusMessage msg = m_dbus->call("getFileNameFromDir", file);
    if (msg.type() == QDBusMessage::ReplyMessage) {
        if (!msg.arguments().isEmpty()) {
            list = msg.arguments().first().toStringList();
        }
    }
    if (list.isEmpty()) {
        list.append(file);
    }
    int tgetlist = m_time->elapsed();
    int count = list.length();
    if (file == "/var/log/syslog") {
        count = qMin(list.length(), detailed);
    }
    QString error;
    for (int i = 0; i < count; i++) {
        QString nowFile = list.at(i);
        QFileInfo info(nowFile);
        if (info.isSymLink()) {
            nowFile = info.canonicalFilePath();
            if (nowFile.isEmpty()) {
                error += list.at(i) + " SymLink error ! \n";
                continue;
            }
        }
        QDBusMessage msg = m_dbus->call("cpFile", nowFile, saveDir);
        if (msg.type() != QDBusMessage::ReplyMessage) {
            qDebug() << "dbus interface error : cpFile" << msg.errorMessage();
            continue;
        }
        if (msg.arguments().isEmpty()) {
            qDebug() << "dbus interface return null : cpFile";
            continue;
        }
        QVariant var = msg.arguments().first();
        if (var.type() == QVariant::String) {
            QString res = var.toString();
            error += res;
        }
    }

    if (!error.isEmpty()) {
        QFile e(saveDir + "error");
        if (e.open(QIODevice::NewOnly)) {
            e.write(error.toLocal8Bit());
            e.close();
        } else {
            qDebug() << "creat error file faild:" << saveDir + "error";
        }
    }
    qDebug() << "获取列表用时：" << tgetlist - tstart << " 采集用时：" << m_time->elapsed() - tgetlist << " 文件数量："
             << count << " | " << message;
}

QString FeedbackManagerLogic::getCmdMessage(const QString &message, bool hasType)
{
    QDBusInterface interface(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_TOOL, dbus::DBUS_INTERFACE_TOOL,
                             QDBusConnection::systemBus());
    QDBusMessage msg = interface.call("getMessage", message);
    if (msg.type() != QDBusMessage::ReplyMessage) {
        qDebug() << "dbus interface error : getMessage" << msg.errorMessage();
        return "";
    }
    if (msg.arguments().isEmpty()) {
        qDebug() << "dbus interface return null : getMessage";
        return "";
    }
    QString res = msg.arguments().first().toString();
    if (res.length() < 1) {
        qDebug() << "dbus interface return string error : getMessage";
        return "";
    }
    if (!hasType) {
        if (res.at(0) == '0') {
            return "";
        }
        res = res.mid(1);
    }
    return res;
}

void FeedbackManagerLogic::collectingCmd(const QString &saveDir, const QString &message)
{
    int tstart = m_time->elapsed();
    QString res = getCmdMessage(message, true);
    if (res.isEmpty()) {
        return;
    }
    QString fileName;
    if (res.at(0) == '0') {
        fileName = "error";
    } else if (res.at(0) == '1') {
        fileName = "message";
    }
    QFile file(saveDir + fileName);
    if (!file.open(QIODevice::NewOnly)) {
        qDebug() << "creat user data file error :" << saveDir + fileName;
        return;
    }
    file.write(res.mid(1).toLocal8Bit());
    file.close();
    qDebug() << "采集用时：" << m_time->elapsed() - tstart << " | " << message;
}

void FeedbackManagerLogic::collectingOther(const QString &saveDir, const QString &message, const QString &name)
{
    int tstart = m_time->elapsed();
    QString fileName = "/var/log/hedron/policy_cache.json";
    QString key1 = "after";
    QString value1 = "";
    QString key2 = "type_code";
    QString value2 = "";
    QString key3 = "dest";
    QString res;
    if (name == "防火墙脚本") {
        value1 = "script";
        value2 = "8401";
        res = getPathFromJson(fileName, key1, value1, key3, key2, value2);
    } else if (name == "外设策略文件") {
        value2 = "85**";
        res = getPathFromJson(fileName, key2, value2, key3);
    } else if (name == "应用运行黑名单文件") {
        value2 = "8205";
        res = getPathFromJson(fileName, key2, value2, key3);
    } else if (name == "应用防卸载文件") {
        value1 = "softwareUninstallDisable";
        res = getPathFromJson(fileName, key1, value1, key3);
    }
    if (!value2.contains('*')) {
        collectingFile(saveDir, res, 1);
    } else {
        QFile file(saveDir + "message");
        if (!file.open(QIODevice::NewOnly)) {
            qDebug() << "creat user data file error :" << saveDir + fileName;
            return;
        }
        file.write(res.toLocal8Bit());
        file.close();
    }
    qDebug() << "采集用时：" << m_time->elapsed() - tstart << " | " << message;
}

QString FeedbackManagerLogic::getPathFromJson(QString fn, QString k1, QString v1, QString keyRes, QString k2,
                                              QString v2)
{
    QString cmd = "cat " + fn;
    QDBusMessage msg = m_dbus->call("getMessage", cmd);
    if (msg.type() != QDBusMessage::ReplyMessage) {
        qDebug() << "dbus interface error : getMessage" << msg.errorMessage();
        return "";
    }
    if (msg.arguments().isEmpty()) {
        qDebug() << "dbus interface return null : getMessage";
        return "";
    }
    QString dbusRes = msg.arguments().first().toString();
    if (dbusRes.length() < 1) {
        qDebug() << "dbus interface return string error : getMessage";
        return "";
    }
    if (dbusRes.at(0) == '0') {
        qDebug() << "cat " + fn + " error : getMessage";
        return "";
    }
    QByteArray json = dbusRes.mid(1).toLocal8Bit();
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isArray() && !doc.isObject()) {
        qDebug() << "json error !";
        return "";
    }
    QJsonArray arr = doc.array();
    QString res;
    for (int i = 0; i < arr.size(); i++) {
        QJsonObject jo = arr[i].toObject();
        if (jo.value(k1).toString() == v1) {
            if (!v2.isEmpty()) {
                if (jo.value(k2).toString() == v2) {
                    return jo.value(keyRes).toString();
                }
            } else {
                return jo.value(keyRes).toString();
            }
        }
        if (v1.contains('*')) {
            QString tmp = v1;
            tmp.remove('*');
            if (jo.value(k1).toString().contains(tmp)) {
                res.append("========" + jo.value(k1).toString() + "========\n");
                for (QString key : jo.keys()) {
                    res.append(key + ":" + jo.value(key).toString() + "\n");
                }
            }
        }
    }
    return res;
}

void FeedbackManagerLogic::onProcFinish(int code)
{
    if (code) {
        qDebug() << "onProcFinish code :" << code;
    }
    qDebug() << "创建压缩包完成：" << m_time->elapsed();
    /* 判断是否开启上传功能 */
    if (!Settings::isUpload()) {
        finish(Success);
        return;
    }
    if (*m_cancel) {
        finish(Cancel);
        return;
    }
    QFileInfo info;
    info.setFile(m_savePath);
    qint64 fileSize = info.size();
    if (fileSize > 1024 * 1024 * 50) {
        emit errorMessage("待上传附件总大小超过限制！");
        finish(Oversize);
        return;
    }
    qDebug() << "准备上传：" << m_savePath;
    //上传
    uploadData();
}
