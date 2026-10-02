#include "feedbackmanagerlogic.h"
#include "unistd.h"
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
#include "kom-configure.h"


FeedbackManagerLogic::FeedbackManagerLogic(const ModuleTypeSet &set,
                                           InformationUserStruct userData,
                                           InformationClassItemList allItems,
                                           const QString &savePath,
                                           bool *cancel,
                                           bool isRetry)
    : m_userData(userData),
      m_typeSet(set),
      m_savePath(savePath),
      m_allItems(allItems),
      m_cancel(cancel),
      m_isRetry(isRetry)
{

}

FeedbackManagerLogic::~FeedbackManagerLogic()
{
    m_proc->deleteLater();
    m_dbus->deleteLater();
    m_networkManager->deleteLater();
}

void FeedbackManagerLogic::setBeFrom(BeFrom type, const QString &key)
{
    m_beFrom = type;
    m_beFromKey = key;
}

void FeedbackManagerLogic::startCollect() {
    emit creatProgress(0);
    m_networkManager = new QNetworkAccessManager;
    if (m_isRetry) {
        uploadData();
        return;
    }

    // 初始化操作
    if (m_time == nullptr) {
        m_time = new QTime;
        m_time->start();
    } else {
        m_time->restart();
    }
    if (m_proc) {
        m_proc->deleteLater();
    }
    m_proc = new QProcess();
    connect(m_proc, &QProcess::readyRead, this, &FeedbackManagerLogic::getProgress);
    connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcFinish(int, QProcess::ExitStatus)));
    m_dbus = new QDBusInterface(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_TOOL, dbus::DBUS_INTERFACE_TOOL,
                                QDBusConnection::systemBus());

    // 创建临时目录
    char host_name[HOST_NAME_MAX + 1];
    memset(host_name, 0, sizeof(host_name));
    if (gethostname(host_name, sizeof(host_name)) == -1) {
        qCritical() << "Generate work order get hostname fail: " << strerror(errno);
        strncpy(host_name, "unknow", HOST_NAME_MAX);
    }
    if (m_userData.uniqueType.isEmpty()) {
        m_userData.uniqueType = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
    }
    m_cacheDir = "/tmp/kom-pfb_" + QString(host_name) + "_" + m_userData.uniqueType + "/";
    m_tmpPath = m_cacheDir + "kylin-os-manager";
    if (!QDir().mkpath(m_tmpPath)) {
        qCritical() << "Generate work order fail: create temp path error.";
        Q_EMIT errorMessage(tr("Failed to create temporary directory!"));
        return;
    };

    m_savePath = m_savePath + "/" + QString("附件_") + QString(host_name) + "_" + m_userData.uniqueType + ".7z";

    // 缓存用户数据
    saveUserData();

    // 缓存调试信息
    saveDebugData();
    qInfo() << "缓存用户数据完成：" << m_time->elapsed();

    // 生成附件
    collecting();
    qInfo() << "采集系统信息完成：" << m_time->elapsed();

    if (*m_cancel) {
        Clear();
        finish(Cancel);
        return;
    }

    // 压缩
    creatPackage();
}

void FeedbackManagerLogic::collecting() {
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
                qCritical() << "Generate work order fail: create annex temp dir error(" << tmpSaveDir << ").";
                continue;
            };

            auto current_time = QTime::currentTime();
            if (it->collectionType() == InformationItem::CollectionType::File &&
                it->getMessage().startsWith("~"))
            {
                auto home_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::HomeLocation);
                auto annex = it->getMessage();
                if (!annex.isEmpty()) {
                    annex = annex.replace(0, 1, home_path);
                    QFileInfo annex_info(annex);
                    auto new_name = QString("%1%2").arg(tmpSaveDir, annex_info.fileName());
                    if (!QFile::copy(annex, new_name)) {
                        qCritical() << "Collect annex(" << annex << ") fail.";
                    }
                }
            } else {
                auto type = Settings::getInfoItemMap().key(it);
                m_dbus->call("collectionMessage", type, tmpSaveDir, cit->Detailed());
            }
            qInfo() << "Time consumption for data collection: " << current_time.secsTo(QTime::currentTime()) << "s(" << it->getMessage() << ").";
        }
        Q_EMIT creatProgress((i + 1) * 25 / classItemNumber);
    }
}

void FeedbackManagerLogic::creatPackage()
{
    m_proc->waitForReadyRead();
    QString cmd = "/usr/lib/p7zip/7za";
    QStringList args;
    args.append("a");
    args.append("-y");
    //openKylin升了7za版本，不接受-l参数了，且原始二进制路径变了，为了保证后续兼容
    if (!QFile(cmd).exists()) {
        cmd = "/usr/bin/7za";
    } else {
        args.append("-l");
    }
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

void FeedbackManagerLogic::saveUserData() {
    QString userPath = m_tmpPath + "/" + "用户提交";
    if (!QDir().mkpath(userPath)) {
        qDebug() << "creat tmp dir error" << userPath;
        return;
    };

    QFile file(userPath + "/提交内容.html");
    if (!file.open(QIODevice::NewOnly)) {
        qDebug() << "creat user data file error";
        return;
    }

    QByteArray info;
    info.append(m_userData.details);
    file.write(info);
    file.close();

    for (const QString &f : m_userData.accessory) {
        QFileInfo file_info(f);
        auto new_path = QString("%1/%2").arg(userPath, file_info.fileName());
        if (!QFile::copy(f, new_path)) {
            qCritical() << "Collect user custom annex fail.";
        }
    }
}

void FeedbackManagerLogic::saveDebugData()
{
    if (m_userData.debugFilePath.isEmpty()) {
        return;
    }
    QFileInfo fi(m_userData.debugFilePath);
    if (!fi.exists()) {
        return;
    }
    QString debugPath = m_tmpPath + "/" + "调试信息";
    if (!QDir().mkpath(debugPath)) {
        qDebug() << "creat tmp dir error" << debugPath;
        return;
    };
    QString cmd = "/usr/bin/cp -r \""+m_userData.debugFilePath+"\" \""+debugPath+"/\"";
    QProcess proc;
    proc.start(cmd);
    proc.waitForFinished();
}

void FeedbackManagerLogic::uploadData()
{
    /* 读取 url 信息 */
    auto [protocol, domain, port] = Settings::getUrlInformation();
    QString baseUrl = QString("%1://%2").arg(protocol, domain);
    if (!port.isEmpty()) {
        baseUrl += ":" + port;
    }
    baseUrl += "/creatbug";

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    appendHttpPart(multiPart, "internolMode", QString::number(m_userData.module));
    appendHttpPart(multiPart, "execution", m_userData.execution);
    appendHttpPart(multiPart, "severity", m_userData.severity);
    appendHttpPart(multiPart, "pri", m_userData.pri);
    appendHttpPart(multiPart, "type", m_userData.bugType);
    appendHttpPart(multiPart, "task", m_userData.task);
    appendHttpPart(multiPart, "story", m_userData.story);
    appendHttpPart(multiPart, "deadline", m_userData.deadline);
    appendHttpPart(multiPart, "project", m_userData.projectId);
    appendHttpPart(multiPart, "assignedTo", m_userData.assignedTo);
    appendHttpPart(multiPart, "case", m_userData.usecase);
    appendHttpPart(multiPart, "product_id", m_userData.productId);

    if (m_userData.level != 0) {
        appendHttpPart(multiPart, "severity", QString::number(m_userData.level));
    }
    if (m_userData.type != 0) {
        appendHttpPart(multiPart, "pri", QString::number(m_userData.type));
    }
    appendHttpPart(multiPart, "openedBuild", m_userData.buildId);
    appendHttpPart(multiPart, "txjg", m_userData.CPUArch);
    appendHttpPart(multiPart, "module", m_userData.zentaoModule);
    appendHttpPart(multiPart, "branch", m_userData.zentaoBranch);

    appendHttpPart(multiPart, "user", m_userData.user);
    appendHttpPart(multiPart, "phoneNumber", m_userData.phoneNumber);
    appendHttpPart(multiPart, "serviceNumber", m_userData.serviceNumber);
    appendHttpPart(multiPart, "priviteTypeStr", m_userData.priviteTypeStr);

    appendHttpPart(multiPart, "title", m_userData.title);
    appendHttpPart(multiPart, "usermail", m_userData.mail);
    appendHttpPart(multiPart, "classtype", m_userData.priviteType);
    appendHttpPart(multiPart, "steps", m_userData.details);
    appendHttpPart(multiPart, "employee_id", m_userData.jobNumber);
    appendHttpPart(multiPart, "full_name", m_userData.userName);
    appendHttpPart(multiPart, "group", m_userData.userGroup);
    appendHttpPart(multiPart, "files", m_savePath);
    appendVerifyHttpPart(multiPart);

    QNetworkRequest creatReq;
    creatReq.setUrl(QUrl(baseUrl));
    QSslConfiguration tmpSSL = creatReq.sslConfiguration();
    tmpSSL.setPeerVerifyMode(QSslSocket::VerifyNone);
    creatReq.setSslConfiguration(tmpSSL);
    creatReq.setRawHeader("username", m_userData.zentaoName.toLocal8Bit());
    creatReq.setRawHeader("password", m_userData.zentaoPwd.toLocal8Bit());
    creatReq.setRawHeader("x-app", "koms-feedback/1.0");
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

void FeedbackManagerLogic::uploadFinish() {
    // FIXME:
    // 在取消流程中，因为是否取消依赖于 m_cache 变量的值，而 m_cache 在多线程中被修改，如果提交流程跑到此处，则意味着提交完成，
    // 不应该再被取消，如果此时另外一个线程发出取消信号，则会等到该函数执行完毕后才会处理取消信号。等于提交显示提交成功后又接着显示了取消成功。
    QByteArray creatByt = m_networkReply->readAll();
    m_networkReply->deleteLater();

    qInfo() << "uploadFinish :" << QString::fromUtf8(creatByt);

    if (m_beFrom == FromDomain) {
        finish(Success, m_beFromKey + "$$$" + creatByt);
        return;
    }

    QVariant idVar = QJsonDocument::fromJson(creatByt).object().value("bugid");
    if (idVar.toString().split(":").last().toInt() > 0) {
        Settings::setHistoryBug(idVar.toString());
        finish(Success, idVar.toString());
    } else if (idVar.toInt() > 0) {
        auto bug_id = QString("%1:%2").arg(m_userData.module).arg(idVar.toInt());
        Settings::setHistoryBug(bug_id);
        finish(Success, bug_id);
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
    if (m_proc && m_proc->state() != QProcess::NotRunning) {
        m_proc->kill();
    }

    Clear();
    finish(Cancel);
}

void FeedbackManagerLogic::finish(FeedBackFinishType type, QString str) {
    if (!m_isRetry) {
        qDebug() << "采集结束，本次总用时：" << m_time->elapsed();
    }

    if (type == Cancel && m_cancelSuccess) {
        return;
    }
    if ((type == UploadFail || type == Refuse) &&
        Settings::getCustomize() == Settings::ICBC &&
        Settings::isUpload())
    {
        // Task: 221695 需求30000 【麒麟管家】使用反馈上报工单系统
        // 上传失败后将压缩包从 /tmp 拷贝到 ~/桌面
        QFileInfo file_info(str);
        if (file_info.exists()) {
            auto file_name = file_info.fileName();
            auto new_path = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DesktopLocation), file_name);
            if (QFile::copy(str, new_path)) {
                Q_EMIT creatFinish(type, new_path);
                return;
            }
            qCritical() << "Copying compressed file from /tmp to desktop fail.";
        } else {
            qCritical() << "The /tmp compressed package does not exist";
        }
    }
    Q_EMIT creatFinish(type, str);
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
#ifdef DEBUG_MODE
        qDebug() << name << value;
#endif
    }

    multiPart->append(part);
}

void FeedbackManagerLogic::appendVerifyHttpPart(QHttpMultiPart *multiPart)
{
    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data;name=\"verify\"");
    QString hmacSha256 =
        QMessageAuthenticationCode::hash(m_posData, Settings::getKeyCode(), QCryptographicHash::Sha256).toBase64();
    part.setBody(hmacSha256.toLocal8Bit());
    multiPart->append(part);
}

void FeedbackManagerLogic::onProcFinish(int code, QProcess::ExitStatus status)
{
    if (*m_cancel) {
        Clear();
        finish(Cancel);
        return;
    }
    ClearCache();

    if (code) {
        qDebug() << "onProcFinish code :" << code << status;
        qDebug() << "onProcFinish error output :" << m_proc->readAllStandardOutput();
        qDebug() << "onProcFinish error message :" << m_proc->errorString();
    }

    qDebug() << "创建压缩包完成：" << m_time->elapsed();

    // 仅生成日志，不上传
    if (m_onlyGenerateLogs) {
        finish(FeedBackFinishType::ExportLogSuccess);
        return;
    }

    // 判断是否开启上传功能
    if (!Settings::isUpload()) {
        finish(Success);
        return;
    }
    if (*m_cancel) {
        finish(Cancel);
        return;
    }

    // 判断附件是否超出大小限制
    int maxSize = 50;
    Settings::Customize customize = Settings::getCustomize();
    if (customize == Settings::ICBC || customize == Settings::INTERNAL) {
        maxSize = 10;
    }

    QFileInfo info;
    info.setFile(m_savePath);
    qint64 fileSize = info.size();
    if (fileSize > 1024 * 1024 * maxSize) {
        emit errorMessage("待上传附件总大小超过限制！");
        finish(Oversize, m_savePath);
        return;
    }

    qDebug() << "准备上传：" << m_savePath << info.exists();

    // 上传
    uploadData();
}

void FeedbackManagerLogic::setOnlyGenerateLogs(bool ok) {
    m_onlyGenerateLogs = ok;
}

void FeedbackManagerLogic::Clear() {
    if (QFile::exists(m_savePath))
        QFile::remove(m_savePath);
    ClearCache();
}

void FeedbackManagerLogic::ClearCache() {
    if (QFile::exists(m_cacheDir)) {
        QDBusInterface interface(dbus::DBUS_SERVICENAME,
                                 dbus::DBUS_PARH_TOOL,
                                 dbus::DBUS_INTERFACE_TOOL,
                                 QDBusConnection::systemBus());
        QString command = QString("rm -rf %1").arg(m_cacheDir);
        QDBusMessage message = interface.call("getMessage", command);
        if (message.type() == QDBusMessage::MessageType::ErrorMessage)
            qDebug() << "Clear cache fail.";
    }
}
