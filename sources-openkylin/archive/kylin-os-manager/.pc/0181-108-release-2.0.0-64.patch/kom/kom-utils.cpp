#include "kom-utils.h"
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include <QWidget>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include "kom-ukui-gsettings.h"

#define PKG_NAME_MAX 128

//安全中心联网加白
struct kysec_netctl_pkg {
    char pkg[PKG_NAME_MAX];
    int type;
    char ico[PATH_MAX];
    char desktop[PATH_MAX];
    int server_type;
};

typedef int (*EnableNetworkingFunc)(uid_t uid, char *pkg, int type, char *ico, char *desktop, int server_type);
typedef struct kysec_netctl_pkg*(*CheckNetworkingFunc)(uid_t uid, char *pkg);

namespace kom {

double KomUtils::adaptFontSize(double pixelSize)
{
    static int defaultPointSize = 10;

    QWidget widget;
    double dpi = widget.logicalDpiX();
    if (dpi < 1)
        dpi = 96.0;
    double currentPointSize = pixelSize * 72 / dpi;

    return currentPointSize / defaultPointSize * UkuiGsettings::getInstance()->getFontSize().toDouble();
}

void KomUtils::setNetworkWhiteListFromPackage(WhiteListType p)
{
    qDebug()<<"开始执行联网控制加白";
    QString libName;
    QDir libDir("/lib");
    QFileInfoList dirlist = libDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &dir : dirlist) {
        QFileInfoList fileList = QDir(dir.absoluteFilePath()).entryInfoList(QStringList()<<"libkysec_extend.so*" , QDir::Files | QDir::NoDotAndDotDot);
        if (!fileList.isEmpty()) {
            libName = fileList.first().absoluteFilePath();
            break;
        }
    }
    if (libName.isEmpty()) {
        qDebug()<<"找不到动态库："<< "libkysec_extend.so";
        return;
    }
    void *securityFd = dlopen(libName.toLocal8Bit().data(), RTLD_LAZY);
    if (!securityFd) {
        char *err = dlerror();
        qDebug()<<"链接动态库失败："<< libName << QByteArray(err);
        return;
    }
    EnableNetworkingFunc fun;
    *(void **) (&fun) = dlsym(securityFd, "kysec_netctl_pkg_add");
    if (fun == NULL) {
        qDebug()<< "找不到符号:kysec_netctl_pkg_add";
        dlclose(securityFd);
        return;
    }
    int res = fun(getuid() , "kylin-os-manager" , 1 , NULL , NULL , p);
    if (res == 0) {
        qDebug() << "添加网络控制白名单成功";
        dlclose(securityFd);
        return;
    }
    //添加失败或已存在该节点
    CheckNetworkingFunc fun2;
    *(void **) (&fun2) = dlsym(securityFd, "kysec_netctl_pkg_read_node_with_uid");
    if (fun2 == NULL) {
        qDebug()<< "找不到符号:kysec_netctl_pkg_read_node_with_uid";
        dlclose(securityFd);
        return;
    }
    kysec_netctl_pkg *pkginfo = fun2(getuid(),"kylin-os-manager");
    if (pkginfo == NULL) {
        qDebug()<<"获取白名单信息失败";
        dlclose(securityFd);
        return;
    }
    if (pkginfo->type == 1) {
        qDebug() << "网络控制白名单中已存在该节点";
        dlclose(securityFd);
        return;
    }
    //存在该节点但权限不是1
    EnableNetworkingFunc fun3;
    *(void **) (&fun3) = dlsym(securityFd, "kysec_netctl_pkg_update");
    if (fun3 == NULL) {
        qDebug()<< "找不到符号:kysec_netctl_pkg_update";
        dlclose(securityFd);
        return;
    }
    int res2 = fun3(getuid() , "kylin-os-manager" , 1 , NULL , NULL , p);
    if (res2 == 0) {
        qDebug()<< "更新白名单信息成功";
        dlclose(securityFd);
        return;
    }
    dlclose(securityFd);
    qDebug() << "添加网络控制白名单失败" << pkginfo->type;
}

/*
 * 先从 /etc/kylin-version/kylin-system-version.conf 文件中获取
 *     os_version     字段为系统版本
 *     update_version 字段为补丁版本
 *
 * 如果上述文件不存在则从 /etc/os-release 文件中获取系统版本
 *     KYLIN_RELEASE_ID 字段为系统版本
 * 从 /usr/share/kylin-update-desktop-config/config/kylin-update-desktop-system.json
 * 或 /usr/share/kylin-update-desktop-config/data/kylin-update-desktop-system.json 文件中获取补丁版本
 *     version 字段为补丁版本
 *
 * 从 /etc/.kyinfo 文件中获取授权版本
 *     milestone 字段为授权版本
 *
 * 从 /etc/kylin-build 文件中获取 build id
 *     buildid 字段为 build id
 */
struct SystemVersion KomUtils::SystemVersion() {
    QString os_version;
    QString update_version;
    QString authorize_version;
    QString build_id;

    if (QFileInfo("/etc/kylin-version/kylin-system-version.conf").exists()) {
        QSettings settings("/etc/kylin-version/kylin-system-version.conf", QSettings::Format::IniFormat);
        os_version = settings.value("SYSTEM/os_version", "").toString();
        update_version = settings.value("SYSTEM/update_version", "").toString();
    }

    if (os_version.isEmpty() && QFileInfo("/etc/os-release").exists()) {
        QSettings settings("/etc/os-release", QSettings::Format::IniFormat);
        os_version = settings.value("KYLIN_RELEASE_ID", "").toString();
    }
    if (update_version.isEmpty()) {
        QFile file;
        if (QFileInfo("/usr/share/kylin-update-desktop-config/config/kylin-update-desktop-system.json").exists()) {
            file.setFileName("/usr/share/kylin-update-desktop-config/config/kylin-update-desktop-system.json");
        } else if (QFileInfo("/usr/share/kylin-update-config/data/kylin-update-desktop-system.json").exists()) {
            file.setFileName("/usr/share/kylin-update-desktop-config/data/kylin-update-desktop-system.json");
        } else {
            file.setFileName("");
        }
        if (file.open(QIODevice::ReadOnly)) {
            const auto data = file.readAll();
            file.close();

            QJsonParseError error;
            const auto doc = QJsonDocument::fromJson(data, &error);
            if (!doc.isNull()) {
                if (doc.isObject()) {
                    const auto root = doc.object();
                    if (root.contains("version")) {
                        update_version = root.value("version").toString();
                    }
                }
            }
        }
    }

    if (QFileInfo("/etc/.kyinfo").exists()) {
        QSettings settings("/etc/.kyinfo", QSettings::Format::IniFormat);
        authorize_version = settings.value("dist/milestone", "").toString();
    }

    QFile file("/etc/kylin-build");
    if (file.open(QIODevice::ReadOnly)) {
        while (!file.atEnd()) {
            auto data = QString::fromUtf8(file.readLine());
            if (data.contains("buildid:")) {
                build_id = data.remove("buildid:").trimmed();
                break;
            }
        }
        file.close();
    }

    struct SystemVersion system_version;
    system_version.os_version = os_version;
    system_version.update_version = update_version;
    system_version.authorize_version = authorize_version;
    system_version.build_id = build_id;

    return system_version;
}

}
