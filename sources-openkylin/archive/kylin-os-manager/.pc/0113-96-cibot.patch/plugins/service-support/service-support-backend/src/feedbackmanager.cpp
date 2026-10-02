#include "feedbackmanager.h"
#include <QDebug>
#include <QFileInfo>
#include <QThread>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaType>
#include <QMessageAuthenticationCode>
#include "feedbackmanagerlogic.h"
#include "settings.h"
#include "informationitem.h"

FeedbackManager *FeedbackManager::m_feedbackManager = nullptr;
FeedbackManager *FeedbackManager::getInstance()
{
    if (m_feedbackManager == nullptr) {
        m_feedbackManager = new FeedbackManager;
    }
    return m_feedbackManager;
}

InformationClassItemList FeedbackManager::getLogFileItems()
{
    return m_allItems;
}

ChooseClassList FeedbackManager::getChooseClassList()
{
    if (!m_chooseItemList.isEmpty()) {
        return m_chooseItemList;
    }
    QString defaultTwo = tr("select detailed category");
    QString defaultTwoCN = "请选择详细分类";
    ChooseClassItem item1;
    item1.code = 1;
    item1.one = tr("System");
    item1.oneCN = "系统使用";
    item1.two << defaultTwo << tr("System activation") << tr("System installation") << tr("System crash")
              << tr("System performance") << tr("Control center") << tr("System setting")
              << tr("System basis consulting");
    item1.twoCN << defaultTwoCN << "系统激活"
                << "系统安装"
                << "系统崩溃"
                << "系统性能"
                << "控制面板"
                << "系统设置"
                << "系统基础咨询";
    item1.guide = tr("Please describe in detail the problem you encountered, such as: unable to activate the system, "
                     "can not find the relevant "
                     "Settings, not clear system features, etc. ");
    m_chooseItemList.append(item1);
    ChooseClassItem item2;
    item2.code = 2;
    item2.one = tr("Peripheral");
    item2.oneCN = "外设咨询";
    item2.two << defaultTwo << tr("Peripheral adaptation consulting") << tr("Peripheral driver acquisition")
              << tr("Peripheral use and error reporting");
    item2.twoCN << defaultTwoCN << "外设适配咨询"
                << "外设驱动获取"
                << "外设使用与报错";
    item2.guide = tr("Please describe in detail the problems you encountered, such as: peripheral connection failure, "
                     "sharing function Settings, "
                     "peripheral adaptation, etc. ");
    m_chooseItemList.append(item2);
    ChooseClassItem item3;
    item3.code = 3;
    item3.one = tr("Application");
    item3.oneCN = "应用软件";
    item3.two << defaultTwo << tr("Software installation and uninstallation") << tr("Software use and error reporting");
    item3.twoCN << defaultTwoCN << "软件安装与卸载"
                << "软件使用与报错";
    item3.guide = tr("Please describe in detail the problems that you encounter, such as obtaining, installing, and "
                     "uninstalling Kirin software "
                     "errors. ");
    m_chooseItemList.append(item3);
    ChooseClassItem item4;
    item4.code = 4;
    item4.one = tr("Other");
    item4.oneCN = "其他";
    item4.two << defaultTwo << tr("Opinions and suggestions");
    item4.twoCN << defaultTwoCN << "意见建议";
    item4.guide = tr("Please describe your problem in detail, or you can also fill in your request or comment here.");
    m_chooseItemList.append(item4);
    return m_chooseItemList;
}

ChooseClassItem FeedbackManager::getChooseClassItem(int code)
{
    for (ChooseClassItem &item : m_chooseItemList) {
        if (item.code == code) {
            return item;
        }
    }
    return ChooseClassItem();
}

void FeedbackManager::setTypes(ModuleTypeSet set)
{
    m_typeSet = set;
}

FeedbackManager::FeedbackManager()
{
    qRegisterNormalizedMetaType<FeedBackFinishType>("FeedBackFinishType");
    m_allItems = Settings::getSettings();
    m_cancel = new bool(false);
    m_thread = new QThread(this);
    m_networkManager = new QNetworkAccessManager(this);
    m_networkManagerTimer = new QTimer(this);
    m_networkManagerTimer->setSingleShot(true);
    auto cus = Settings::getCustomize();
    if (cus == Settings::ICBC || cus == Settings::INTERNAL) {
        m_internalMode = ProjectMode;
    }
    connect(m_networkManagerTimer, &QTimer::timeout, this, [=] {
        qDebug() << "======超时======";
        if (m_reply) {
            getNetWorkDataPri(m_reply);
            m_reply->abort();
            m_reply->deleteLater();
        }
    });
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &FeedbackManager::getNetWorkDataPri);
    m_thread->start();
}

void FeedbackManager::setUserData(const InformationUserStruct &u)
{
    m_userData = u;
}

void FeedbackManager::setSavePath(const QString &path)
{
    m_savePath = path;
}

void FeedbackManager::setOnlyGenerateLogs(bool ok) {
    m_onlyGenerateLogs = ok;
}

void FeedbackManager::startCollecting()
{
    //    if (m_userData.title.isEmpty() || m_userData.originalDetails.isEmpty()) {
    //        emit errorMessage(tr("Title and question details cannot be blank!"));
    //        return;
    //    }
    //    qint64 fileSize = 0;
    //    for (const QString &file : m_userData.accessory) {
    //        QFileInfo info;
    //        info.setFile(file);
    //        fileSize += info.size();
    //    }
    //    if (fileSize > 1024 * 1024 * 20) {
    //        emit errorMessage(tr("Attachment size exceeds limit!"));
    //        return;
    //    }
    *m_cancel = false;
    FeedbackManagerLogic *logic = new FeedbackManagerLogic(m_typeSet, m_userData, m_allItems, m_savePath, m_cancel);
    logic->setBeFrom(m_beFrom, m_beFromKey);
    logic->setOnlyGenerateLogs(m_onlyGenerateLogs);
    m_savePath.clear();
    m_packageFile.clear();
    connect(m_feedbackManager, &FeedbackManager::cancelSignal, logic, &FeedbackManagerLogic::cancel);
    connect(m_feedbackManager, &FeedbackManager::startCollect, logic, &FeedbackManagerLogic::startCollect);
    connect(logic, &FeedbackManagerLogic::errorMessage, this, &FeedbackManager::errorMessage);
    connect(logic, &FeedbackManagerLogic::creatProgress, this, &FeedbackManager::creatProgress);
    connect(logic, &FeedbackManagerLogic::creatFinish, this, [=](FeedBackFinishType type, QString str) {
        if (type == UploadFail) {
            m_packageFile = str;
        }
        emit creatFinish(type, str);
    });
    connect(logic, &FeedbackManagerLogic::creatFinish, logic, &FeedbackManagerLogic::deleteLater);
    logic->moveToThread(m_thread);
    emit startCollect();
}

void FeedbackManager::retryUpload()
{
    if (m_packageFile.isEmpty()) {
        return;
    }
    qDebug() << "=======重试上传=======";
    *m_cancel = false;
    FeedbackManagerLogic *logic =
        new FeedbackManagerLogic(m_typeSet, m_userData, m_allItems, m_packageFile, m_cancel, true);
    logic->setBeFrom(m_beFrom, m_beFromKey);
    connect(m_feedbackManager, &FeedbackManager::cancelSignal, logic, &FeedbackManagerLogic::cancel);
    connect(m_feedbackManager, &FeedbackManager::startCollect, logic, &FeedbackManagerLogic::startCollect);
    connect(logic, &FeedbackManagerLogic::creatProgress, this, &FeedbackManager::creatProgress);
    connect(logic, &FeedbackManagerLogic::creatFinish, this, &FeedbackManager::creatFinish);
    connect(logic, &FeedbackManagerLogic::creatFinish, logic, &FeedbackManagerLogic::deleteLater);
    logic->moveToThread(m_thread);
    emit startCollect();
}

void FeedbackManager::closeBug(QString id)
{
    m_netWorkType = CloseBug;
    QByteArray bugId = id.toLocal8Bit();
    startGetPri(bugId, "closebug", true);
}

QString FeedbackManager::getGroup()
{
    QFile file("/etc/kylin-region");
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Get group open file fail";
        return "";
    }
    QString result = file.readAll();
    file.close();

    qInfo() << "Current group: " << result;

    return result;
}

void FeedbackManager::getProjectInfo(const QString &name, const QString &pwd)
{
    m_zentaoName = name;
    m_zentaoPwd = pwd;
    QString result;
    QFile file("/etc/kylin-build");
    if (!file.open(QIODevice::ReadOnly)) {
       result = "";
    } else {
        result = file.readAll();
        file.close();
    }
    QStringList tmpList = result.split("\n");
    QString buildId;
    for (QString &info : tmpList) {
        if (info.contains("buildid")) {
            buildId = buildId.split(" ").last();
            buildId.chop(1);
            break;
        }
    }
    m_netWorkType = GetProjectInfo;
    startGetPri(buildId.toLocal8Bit(), "kylinbuild", false);
}

void FeedbackManager::setBeFrom(BeFrom type, const QString &key)
{
    m_beFrom = type;
    m_beFromKey = key;
}

void FeedbackManager::getHistoryData(int page)
{
    if (page < 1) {
        qDebug() << "页数指定错误！";
        return;
    }
    int onePageItems = 10;
    QString setting = Settings::getHistoryBug();
    if (setting.isEmpty()) {
        qDebug() << "没有历史提交！";
        return;
    }
    m_PageIndex = page;
    m_netWorkType = BugInfo;
    QString result;
    QStringList tmp = setting.split(",");
    tmp.removeAll("");
    int nowNumber = 0;
    int start = onePageItems * (page - 1);
    for (int i = start; i < tmp.length(); i++) {
        if (nowNumber >= onePageItems) {
            break;
        }
        QString assembly = tmp.at(i);
        if (assembly.split(":").first() != QString::number(m_internalMode)) {
            continue;
        }
        result += assembly + ",";
        nowNumber++;
    }
    if (result.isEmpty()) {
        qDebug() << "没有该模式下的历史提交！" << m_internalMode;
        return;
    }
    result.chop(1);
    startGetPri(result.toLocal8Bit(), "getbuginfo", true);
}

void FeedbackManager::cancel()
{
    *m_cancel = true;
    emit cancelSignal();
}

void FeedbackManager::getNetWorkDataPri(QNetworkReply *reply)
{
    m_networkManagerTimer->stop();
    QByteArray byte = reply->readAll();
    reply->deleteLater();
    if (m_netWorkType == BugInfo) {
        getHistoryDataPri(byte);
    } else if (m_netWorkType == CloseBug) {
        getCloseBugDataPri(byte);
    } else if (m_netWorkType == GetProjectInfo) {
        getProjectInfoDataPri(byte);
    }
}

void FeedbackManager::getHistoryDataPri(const QByteArray &byte)
{
    QList<HistoryInfo> result;
    QJsonArray list = QJsonDocument::fromJson(byte).object().value("buginfo").toArray();
    for (const QJsonValue bugs : list) {
        if (!bugs.isObject()) {
            continue;
        }
        QJsonObject obj = bugs.toObject();
        HistoryInfo res;
        QString openedDate = obj.value("openedDate").toString();
        res.creatTime = QDateTime::fromString(openedDate, Qt::ISODate).toLocalTime();
        QString title = obj.value("title").toString();
        QStringList tmp = title.split("|");
        QString type = tmp.first().split("】").last();
        for (ChooseClassItem &item : m_chooseItemList) {
            if (item.oneCN == type) {
                type = item.one;
                break;
            }
        }
        res.type = type;
        res.message = tmp.last();
        QString resolution = obj.value("resolution").toString();
        res.solution = resolution;
        QString active = obj.value("status").toString();
        res.progress = active;
        QStringList buglist = Settings::getHistoryBug().split(",");
        QString id = QString::number(obj.value("id").toInt());
        for (QString &str : buglist) {
            QStringList tmpList = str.split(":");
            if (tmpList.last() == id) {
                id = str;
                break;
            }
        }
        res.id = id;
        if (res.id.isEmpty()) {
            res.type = "?";
            res.message = "?";
            res.solution = "?";
            res.progress = "?";
        }
        result.append(res);
    }
    emit historyInfo(result);
}

void FeedbackManager::getCloseBugDataPri(const QByteArray &byte)
{
    QString result = QJsonDocument::fromJson(byte).object().value("status").toString();
    if (result == "error") {
        qDebug() << "======关闭失败======";
    }
    getHistoryData(m_PageIndex);
}

void FeedbackManager::getProjectInfoDataPri(const QByteArray &byte)
{
    QList<ProjectInfo> result;
    if (byte.isEmpty()) {
        emit prijectInfo(ConnectFaild, result);
        return;
    }
    if (byte.contains("Failed to login to zentao")) {
        emit prijectInfo(LoginError, result);
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(byte);
    if (!doc.isObject()) {
        emit prijectInfo(Nodata, result);
        return;
    }
    if (!doc.isObject()) {
        emit prijectInfo(Nodata, result);
        return;
    }
    QJsonValue tmp = doc.object().value("zentao_builds");
    if (!tmp.isArray()) {
        emit prijectInfo(Nodata, result);
        return;
    }
    QJsonArray arr = tmp.toArray();
#ifdef DEBUG_MODE
//    qDebug() << "FeedbackManager::getProjectInfoDataPri: " << arr;
#endif
    for (int i = 0; i < arr.count(); i++) {
        QJsonObject obj = arr.at(i).toObject();
        ProjectInfo info;
        info.zentaoBuildId = QString::number(obj.value("id").toInt());
        info.zentaoBuildName = obj.value("name").toString();
        info.productId = QString::number(obj.value("product").toInt());
        info.productName = obj.value("productName").toString();
        info.projectId = QString::number(obj.value("project").toInt());
        info.executionId = QString::number(obj.value("execution").toInt());
        info.executionName = obj.value("executionName").toString();
        QMap<QString, QString> branch;
        QJsonArray branchs = obj.value("branchs").toArray();
        for (const QJsonValue va : branchs) {
            QStringList tmp = va.toString().split(":");
            if (tmp.length() < 2) {
                continue;
            }
            branch.insert(tmp.first(), tmp.last());
        }
        info.branchs = branch;
        QMap<QString, QString> module;
        QJsonArray modules = obj.value("modules").toArray();
        for (const QJsonValue va : modules) {
            QStringList tmp = va.toString().split(":");
            if (tmp.length() < 2) {
                continue;
            }
            module.insert(tmp.first(), tmp.last());
        }
        info.modules = module;
        result.append(info);
    }
    if (result.isEmpty()) {
        emit prijectInfo(Nodata, result);
        return;
    }
    m_internalMode = InZentao;
    emit internalModeChange(m_internalMode);
    emit prijectInfo(HasData, result);
}

void FeedbackManager::startGetPri(const QByteArray &byte, const QString &address, bool encipher)
{
    auto [protocol, domain, port] = Settings::getUrlInformation();
    QNetworkRequest creatReq;
    QString baseUrl = QString("%1://%2").arg(protocol).arg(domain);
    if (!port.isEmpty()) {
        baseUrl += ":" + port;
    }
    baseUrl += "/" + address + "/" + byte;
    if (encipher) {
        QString hmacSha256 =
            QMessageAuthenticationCode::hash(byte, Settings::getKeyCode(), QCryptographicHash::Sha256).toHex();
        baseUrl += "/" + hmacSha256;
    }
    if (!m_zentaoName.isEmpty() && !m_zentaoPwd.isEmpty()) {
        creatReq.setRawHeader("username", m_zentaoName.toLocal8Bit());
        creatReq.setRawHeader("password", m_zentaoPwd.toLocal8Bit());
    }
#ifdef DEBUG_MODE
    qDebug() << baseUrl;
#endif
    creatReq.setUrl(QUrl(baseUrl));
    QSslConfiguration tmpSSL = creatReq.sslConfiguration();
    tmpSSL.setPeerVerifyMode(QSslSocket::VerifyNone);
    creatReq.setSslConfiguration(tmpSSL);
    m_reply = m_networkManager->get(creatReq);
    m_networkManagerTimer->start(15000); // 15秒超时
}
