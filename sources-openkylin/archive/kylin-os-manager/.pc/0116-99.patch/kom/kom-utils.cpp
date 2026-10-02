#include <QWidget>
#include <QDebug>
#include "kom-ukui-gsettings.h"
#include "kom-utils.h"

//安全中心联网加白
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <QDir>
#define PKG_NAME_MAX 128
struct kysec_netctl_pkg {
    char pkg[PKG_NAME_MAX];
    int type;
    char ico[PATH_MAX];
    char desktop[PATH_MAX];
    int server_type;
};
typedef int (*EnableNetworkingFunc)(uid_t uid, char *pkg, int type, char *ico, char *desktop, int server_type);
typedef struct kysec_netctl_pkg*(*CheckNetworkingFunc)(uid_t uid, char *pkg);

namespace kom
{

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

} // namespace kom
