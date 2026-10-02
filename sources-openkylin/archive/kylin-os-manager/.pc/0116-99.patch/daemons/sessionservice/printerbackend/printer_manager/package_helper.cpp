#include "package_helper.h"
#include "common.h"
#include <QThread>
#include <QTimer>


bool packageIsInstalled(const QString &debName, const QString &packageName)
{
    // brscan5chn_1.2.5-0_amd64.deb
    QString packageVersion = getPackageVersion(packageName);
    if (packageVersion.isEmpty()) {
        return false;
    }
    QString deb(debName);
    // perhaps deb is brscan5chn
    if (!deb.contains("_")) {
        if (deb == packageName) {
            return true;
        } else {
            return false;
        }
    }

    deb.remove(0, deb.indexOf("_") + 1);

    if (!deb.contains("_")) {
        return false;
    }

    QString debVersion = deb.left(deb.indexOf("_"));
    if (debVersion.isEmpty()) {
        return false;
    }
    if (debVersion != packageVersion) {
        return false;
    }
    return true;
}


PackageHelper::PackageHelper(DeviceInformation &info, QObject *parent)
    : m_info(info), QObject(parent)
{
    m_thread = new QThread();
    moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &PackageHelper::dowork);

    connect(this, &PackageHelper::finished, m_thread, &QThread::quit);
    connect(this, &PackageHelper::finished, this, &PackageHelper::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

PackageHelper::~PackageHelper()
{

}

void PackageHelper::dowork()
{
    getPackagesNameFromHttp(m_info);

    if (m_info.packageNameList.size() == 0) {
        failed();
        return ;
    }

    int needInstall = false;
    for (int i = 0; i < m_info.packageNameList.size(); i++) {
        if (packageIsInstalled(m_info.debNameList[i], m_info.packageNameList[i]) == false) {
            needInstall = true;
            break;
        }
    }

    succeed(needInstall);
}
