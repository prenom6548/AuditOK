#ifndef MODELDOWNLOAD_H
#define MODELDOWNLOAD_H

#include <QObject>
#include <QApplication>
#include <QMessageBox>
#include <QThread>
#include <QApt/Package>
#include <QApt/Config>
#include <QApt/DebFile>
#include <QApt/Backend>
#include <QApt/Transaction>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QDBusPendingCallWatcher>

#include "kaglobal.h"

#include <ukui-log4qt.h>

namespace QApt {
    class Backend;
    class Package;
    class Transaction;
    class DownloadProgress;
}
class ModelDownload : public QObject
{
    Q_OBJECT
public:

    static ModelDownload *getInstance();
    PkgInstallStatus getPkgStatus(QString pkgName);
    QString getCurPkgVersion(QString pkgName);
    QString getLastestVersion(QString pkgName);
    bool isPkgInstalled(QString pkgName);
    bool downloadModelPkg(const QString pkgName);
    bool getDownloadingStatus();
    void setDownloadingStatus(bool downloading);
    void setIsStartDownloading(bool isStartDownloading);
    bool getStartDownloadingStatus();
    bool getCurrentInstallStatus();
    void setInstallingStatus(bool installing);
    bool cancelDownloading();
    void initQaptBackend();
    QString isAppversionNeedUpdate();
    bool compareCurrentPkgVersion(QString curVersion,QString tarVersion);
signals:
    void sigUpdateModelInstallStatus(int,QString,QString);
    void sigUpdateModelInstallFinished(bool isSuccess,QStringList upgradeGroup,QString errorStr,QString errorDesc);
    void sigUpdateModelDownloadFinished();
    void sigCancelExitButton(bool isInstalling);
public slots:
    void slUpdateModelInstallStatus(int value , QString status, QString details);
    void slUpdateModelInstallFinished(bool isSuccess,QStringList upgradeGroup,QString errorStr,QString errorDesc);
    void slUpdateModelInstallFlag(bool cancelDownloadFlag);
private:
    explicit ModelDownload(QObject *parent = nullptr);
    ModelDownload(const ModelDownload &other) =delete;
    ModelDownload &operator =(const ModelDownload &other) =delete;
    //实例
    static ModelDownload *m_instance;
private:
    QApt::Backend *m_backend;
    std::atomic_bool qaptInited{false};
private:
    QDBusInterface *m_dbusDownloadInterface = nullptr;
    int m_currentValue = 0;
    bool isDownloading = false;
    bool m_isInstalling = false;
    bool m_isStartDownloading = false;
};

#endif // MODELDOWNLOAD_H
