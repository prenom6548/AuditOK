#include "modeldownload.h"

#include "util.h"

#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVersionNumber>


ModelDownload *ModelDownload::m_instance = nullptr;


ModelDownload *ModelDownload::getInstance()
{
    if(!m_instance)
    {
        m_instance = new ModelDownload();
    }
    return m_instance;
}

PkgInstallStatus ModelDownload::getPkgStatus(QString pkgName)
{
    QCoreApplication::processEvents();
    PkgInstallStatus status;
    QString curVersion = getCurPkgVersion(pkgName);
    QString lastVersion = getLastestVersion(pkgName);
    bool isInstalled = isPkgInstalled(pkgName);
    KyInfo()<<"pkgName=====:"<<pkgName <<"curVersion:"<< curVersion <<"lastVersion:"<< lastVersion <<"isinstalled:"<<isInstalled;

    //模型未找到
    if(curVersion == "PkgNotFound")
    {
       status = PkgInstallStatus::PkgNotFound;
       return status;
    }
    //模型已查找到未安装
    if(isInstalled == false && curVersion.isEmpty())
    {
        status = PkgInstallStatus::PkgFoundNotInstall;
        return status;
    }
    //模型已经下载并且版本是最新的
    if(lastVersion.isEmpty() && !curVersion.isEmpty())
    {
        status = PkgInstallStatus::PkgLastestVersion;
        return status;
    }
    //模型已安装旧版本发现新版本可更新
    else {

        status = PkgInstallStatus::PkgFoundNewVersion;
        return status;
    }
}

QString ModelDownload::getCurPkgVersion(QString pkgName)
{
    QCoreApplication::processEvents();
    initQaptBackend();
    if(!m_backend)
    {
        KyInfo() << "qapt backend init failed ,can not load package message .";
        return QString();
    }
    auto package  = m_backend->package(pkgName);
    if(package == nullptr){
        QString msg = "PkgNotFound";
        return msg;
    }
    if(package->isInstalled()){
        return package->version();
    }else {
        return QString();
    }
}

QString ModelDownload::getLastestVersion(QString pkgName)
{
    initQaptBackend();
    if(!m_backend)
    {
        KyInfo() << "qapt backend init failed ,can not load package message .";
        return QString();
    }
    auto updatePackageList = m_backend->upgradeablePackages();
    for (auto tmpPkg : updatePackageList) {
        QCoreApplication::processEvents();
        if (tmpPkg->name() == pkgName) {
            return tmpPkg->availableVersion();
        }
    }
    return QString();
}

bool ModelDownload::isPkgInstalled(QString pkgName)
{
    QCoreApplication::processEvents();
    initQaptBackend();
    if(!m_backend)
    {
        KyInfo() << "qapt backend init failed ,can not load package message .";
        return false;
    }
    auto package  = m_backend->package(pkgName);
    if(package == nullptr){
        return false;
    }
    return package->isInstalled();
}

bool ModelDownload::downloadModelPkg(const QString pkgName)
{
    if (m_dbusDownloadInterface == nullptr){
        m_dbusDownloadInterface = new QDBusInterface( "com.kylin.systemupgrade","/com/kylin/systemupgrade"  ,
                                                        "com.kylin.systemupgrade.interface", QDBusConnection::systemBus(),this);
    }
    if(m_dbusDownloadInterface->isValid())
    {
    QStringList downloadDebNameList;
    downloadDebNameList.append(pkgName);
    QVariantList list;
    list << downloadDebNameList;
    QDBusPendingCall replyCall = m_dbusDownloadInterface->asyncCall("InstallPackages", downloadDebNameList);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(replyCall);
    connect(m_dbusDownloadInterface,SIGNAL(InstalldebStatusChanged(int,QString,QString)),this,SLOT(slUpdateModelInstallStatus(int,QString,QString)),Qt::UniqueConnection);
    connect(m_dbusDownloadInterface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(slUpdateModelInstallFinished(bool,QStringList,QString,QString)),Qt::UniqueConnection);
    connect(m_dbusDownloadInterface,SIGNAL(Cancelable(bool)),this,SLOT(slUpdateModelInstallFlag(bool)));
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this,&ModelDownload::sigUpdateModelDownloadFinished,Qt::UniqueConnection);
    return true;
    }
    else
    {
        QMessageBox::warning(nullptr,tr(""),tr("The current environment's dbus interface is abnormal"));
        return false ;
    }
}

bool ModelDownload::getDownloadingStatus()
{
    return  isDownloading;
}

void ModelDownload::setDownloadingStatus(bool downloading)
{
    isDownloading = downloading;
}

void ModelDownload::setIsStartDownloading(bool isStartDownloading)
{
    m_isStartDownloading = isStartDownloading;
}

bool ModelDownload::getStartDownloadingStatus()
{
    return m_isStartDownloading;
}

bool ModelDownload::getCurrentInstallStatus()
{
    return m_isInstalling;
}

void ModelDownload::setInstallingStatus(bool installing)
{
    m_isInstalling = installing;
}

bool ModelDownload::cancelDownloading()
{
    if(!isDownloading)
    {
        return true;
    }
    if (m_dbusDownloadInterface == nullptr){
        m_dbusDownloadInterface = new QDBusInterface( "com.kylin.systemupgrade","/com/kylin/systemupgrade"  ,
                                                        "com.kylin.systemupgrade.interface", QDBusConnection::systemBus(),this);
    }
    if(m_dbusDownloadInterface->isValid())
    {
        //执行取消下载
        QDBusReply<bool> reply = m_dbusDownloadInterface->asyncCall("CancelDownload");
        if (reply.isValid()) {
            if(reply){
                KyInfo() << "cancel download successful:";
                return true;
            }
            else {
                KyInfo() << "Cancel download failed:" << reply.error().message();
                return false;
            }
        } else {
            KyInfo() << "Failed to call CancelDownload:" << reply.error().message();
            return false;
        }
    }
}

void ModelDownload::initQaptBackend()
{
    if(m_backend)
    {
        m_backend->deleteLater();
        m_backend = nullptr;
        m_backend = new QApt::Backend(this);
        m_backend->init();
    }
}

QString ModelDownload::isAppversionNeedUpdate()
{
    // V11 调用 kaiming DBus 接口获取 kylin-aiassistant 版本判断兼容性
    // V11 只需要检查助手兼容性
    if (curSystemVersion() == SystemVersion::Version_V11) {
        // 如果系统版本为空（新系统），通过 QApt 判断助手版本是否小于 3.0.0.0-ok24update1
        const QString osVersion = kdk_system_get_version_detaile().os_version;
        KyInfo() << "isAppversionNeedUpdate: os_version=[" << osVersion << "] isEempty=" << osVersion.isEmpty();
        if (osVersion.isEmpty()) {
            const QString curVer = this->getCurPkgVersion(kaglobal::aiAssistantPkgName);
            const QString minVer = QStringLiteral("3.0.0.0-ok0.24update1");
            bool needUpdate = !curVer.isEmpty() && (curVer < minVer);
            KyInfo() << "isAppversionNeedUpdate[empty system version]: curVer=[" << curVer << "] minVer=[" << minVer << "] needUpdate=" << needUpdate;
            if (needUpdate) {
                return tr("The 'kylin-aiassistant' version is outdated and incompatible with the subsystem, which may result in limited functionality. Please upgrade to the latest version to ensure proper usage.");
            }
            return QString();
        }

        const QString service = QStringLiteral("org.kylin.kaiming");
        const QString objectPath = QStringLiteral("/org/kylin/kaiming");
        const QString interfaceName = QStringLiteral("org.kylin.kaiming.interface");
        const QString name = QStringLiteral("cn.kylin.kylin-aiassistant");

        QDBusInterface interface(service, objectPath, interfaceName, QDBusConnection::systemBus());
        QDBusReply<QString> reply = interface.call(QStringLiteral("getLocalInfo"), name);

        if (!reply.isValid()) {
            KyWarning() << "kaiming getLocalInfo failed:" << interface.lastError().message();
            return QString();
        }

        const QString jsonStr = reply.value();
        const QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (!doc.isObject()) {
            KyWarning() << "kaiming getLocalInfo invalid json:" << jsonStr;
            return QString();
        }

        const QJsonObject obj = doc.object();
        const QString versionStr = obj.value(QStringLiteral("version")).toString();
        const QString archStr = obj.value(QStringLiteral("arch")).toString();
        KyInfo() << "v11 kylin-aiassistant info:" << "arch" << archStr << "version" << versionStr;

        const QVersionNumber cur = QVersionNumber::fromString(versionStr);
        const QVersionNumber min = QVersionNumber::fromString(QStringLiteral("3.0.73"));

        const bool needUpdate =
            (cur.isNull() || min.isNull()) ? (versionStr < QStringLiteral("3.0.73")) : (cur < min);

        if (needUpdate) {
            return tr("The 'kylin-aiassistant' version is outdated and incompatible with the subsystem, which may result in limited functionality. Please upgrade to the latest version to ensure proper usage.");
        }
        return QString();
    }

    //ai助手是否需要更新
    bool aiassistantFlag = this->compareCurrentPkgVersion(this->getCurPkgVersion(kaglobal::aiAssistantPkgName),kaglobal::aiAssistantMinVersion);
    //麒麟速记是否需要更新
    bool aiNoteFlag = compareCurrentPkgVersion(this->getCurPkgVersion(kaglobal::aiNotePkgName),kaglobal::aiNoteMinVersion);
    //如果两个应用的版本
    QString tips = "";
    if(aiassistantFlag == true && aiNoteFlag == false)
    {
        //提示助手需要更新
        tips = tr("The 'kylin-aiassistant' version is outdated and incompatible with the subsystem, which may result in limited functionality. Please upgrade to the latest version to ensure proper usage.");
    }
    else if(aiassistantFlag == false && aiNoteFlag == true)
    {
        //提示速记需要更新
        tips = tr("The 'kylin-note' version is outdated and incompatible with the subsystem, which may result in limited functionality. Please upgrade to the latest version to ensure proper usage.");
    }
    else if(aiassistantFlag == true && aiNoteFlag == true)
    {
        //提示助手和速记都需要更新
        tips = tr("The 'kylin-aiassistant,kylin-note' version is outdated and incompatible with the subsystem, which may result in limited functionality. Please upgrade to the latest version to ensure proper usage.");
    }
    else if(aiassistantFlag == false && aiNoteFlag == false)
    {
        //都不需要更新
        tips = "";
    }
    KyInfo()<< "fffffffffffffffffffffffffffffffff:是否需要显示软件包提示信息:" <<tips << "aiassistantFlag" << aiassistantFlag << "aiNoteFlag" << aiNoteFlag ;
    return tips;
}

bool ModelDownload::compareCurrentPkgVersion(QString curVersion, QString tarVersion)
{
    KyInfo() << "dddddddddddddddddddddddddddddddddd:当前包版本及目标版本" << curVersion <<  ":"<<tarVersion;
    //检查当前应用是否需要提示更新，如果软件为空，则无需提示更新
    if(curVersion.isEmpty())
    {
        return false;
    }
    //当前版本如果小于指定版本则需要返回为true，提醒用户去更新
    bool result = !(curVersion > tarVersion);
    return result;
}

void ModelDownload::slUpdateModelInstallStatus(int value, QString status, QString details)
{
     KyInfo() << "current model downloading value : "<< value;
     m_currentValue = value;
     emit sigUpdateModelInstallStatus(value,status,details);
}

void ModelDownload::slUpdateModelInstallFinished(bool isSuccess, QStringList upgradeGroup, QString errorStr, QString errorDesc)
{
    KyInfo() << "status " << isSuccess <<upgradeGroup << errorStr << errorDesc;
    emit sigUpdateModelInstallFinished(isSuccess,upgradeGroup,errorStr,errorDesc);
}

void ModelDownload::slUpdateModelInstallFlag(bool cancelDownloadFlag)
{
    KyInfo() << "current model cancelDownloadFlag:" << cancelDownloadFlag;
    //如果不可取消下载，则通知前端隐藏取消下载按钮
    if(cancelDownloadFlag == false && (m_currentValue > 50 || m_currentValue == 50))
    {
        m_isInstalling = true;
        KyInfo() << "current model is installing ,could not cancel download";
        emit sigCancelExitButton(m_isInstalling);
    }
}

ModelDownload::ModelDownload(QObject *parent)
{
    QCoreApplication::processEvents();
    m_backend = new QApt::Backend(this);
    if(!m_backend->init()){
        QMessageBox::warning(nullptr,tr("Tips"),tr("Backend init failed"));
    }
    qaptInited = true;
}
