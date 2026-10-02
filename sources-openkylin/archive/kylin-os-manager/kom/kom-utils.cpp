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
#include <QProcess>
#include <QGuiApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <libkylog.h>
#include <libkypackages.h>
#include <windowmanager/windowmanager.h>
#include <singleapplication.h>
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

double KomUtils::adaptFontSize(double pixelSize) {
    static int defaultPointSize = 10;

    QWidget widget;
    double dpi = widget.logicalDpiX();
    if (dpi < 1)
        dpi = 96.0;
    double currentPointSize = pixelSize * 72 / dpi;

    return currentPointSize / defaultPointSize * UkuiGsettings::getInstance()->getFontSize().toDouble();
}

void KomUtils::setNetworkWhiteListFromPackage(WhiteListType p) {
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

void KomUtils::LogOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    const char *message = localMsg.constData();
    const char *file = context.file? context.file: "";
    const char *func = context.function? context.function: "";

    switch (type) {
    case QtDebugMsg:
        klog_debug("%s (%s:%u,%s)\n", message, file, context.line, func);
        break;
    case QtInfoMsg:
        klog_info("%s (%s:%u,%s)\n", message, file, context.line, func);
        break;
    case QtWarningMsg:
        klog_warning("%s (%s:%u,%s)\n", message, file, context.line, func);
        break;
    case QtCriticalMsg:
        klog_err("%s (%s:%u,%s)\n", message, file, context.line, func);
        break;
    case QtFatalMsg:
        klog_emerg("%s (%s:%u,%s)\n", message, file, context.line, func);
        break;
    default:
        break;
    }
}

bool KomUtils::ExistsFullScreenWindow() {
    // 获取所有的窗口 id
    QProcess process;
    process.setProgram("xprop");
    process.setArguments({"-root", "_NET_CLIENT_LIST"});
    process.start();
    if (!process.waitForFinished(5000)) {
        qWarning() << "obtain full screen exec xprop -root failed or timed out";
        return false;
    }

    auto output = QString(process.readAllStandardOutput()).trimmed();
    process.close();

    auto index = output.indexOf('#');
    if (index == -1) {
        qCritical() << "obtain full screen xprop -root output result not found '#'";
        return false;
    }
    if (output.size() <= index + 1) {
        qWarning() << "obtain full screen xprop -root output result no characters after '#'";
        return false;
    }
    auto ids_str = output.mid(index + 1).trimmed();
    auto ids = ids_str.split(',');

    // 获取每个窗口的信息，判断是否是全屏窗口
    for (const auto &id: ids) {
        QProcess p;
        p.setProgram("xprop");
        p.setArguments({"-id", id.trimmed()});
        p.start();
        if (!p.waitForFinished(5000)) {
            qWarning() << "obtain full screen exec xprop -id failed or timed out, id: " << id.trimmed();
            continue;
        }

        auto window_info =  QString(p.readAllStandardOutput());
        p.close();

        if (window_info.contains("_NET_WM_STATE_FULLSCREEN")) {
            return true;
        }
        if (window_info.contains("_NET_WM_STATE_MAXIMIZED_VERT") &&
            window_info.contains("_NET_WM_STATE_MAXIMIZED_HORZ"))
        {
            return true;
        }
    }

    return false;
}

bool KomUtils::LiveEnv() {
    QFile file("/proc/cmdline");
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "[KomUtils][LiveEnv] fail to open file";
        return false;
    }
    auto content = QString::fromUtf8(file.readAll());
    file.close();

    if (content.contains("boot=casper")) {
        return true;
    }

    return false;
}

bool KomUtils::WaylandPlatform() {
    const auto platform = QGuiApplication::platformName();
    qInfo() << "current platform: " << platform;
    if (platform.startsWith("wayland", Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

void KomUtils::SetWaylandActivationWindow(kdk::QtSingleApplication &app) {
    static kdk::WindowId wayland_mainwindow_id;
    if (kom::KomUtils::WaylandPlatform()) {
        QObject::connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, [](const kdk::WindowId &window_id) {
            if (getpid() == (int)kdk::WindowManager::getPid(window_id)) {
                wayland_mainwindow_id = window_id;
            }
        });
        QObject::connect(&app, &kdk::QtSingleApplication::messageReceived, [](QString message) {
            kdk::WindowManager::activateWindow(wayland_mainwindow_id);
        });
    }
}

bool KomUtils::IsPackageInstalled(const QString &package_name) {
#ifdef V11_ENVIRONMENT
    std::string name = package_name.toStdString();
    return (kdk_package_is_installed(name.c_str(), NULL) == 0);
#else
    QProcess process;
    process.setProgram("dpkg");
    QList<QString> args{"-l", package_name};
    process.setArguments(args);
    process.start();
    process.waitForFinished();

    auto output = QString::fromLocal8Bit(process.readAll());
    auto lines = output.split('\n');
    lines.removeAll("");
    if (lines.empty()) {
        return false;
    }

    auto last_line = lines.back().trimmed();
    auto fields = last_line.split(' ');
    fields.removeAll("");
    if (fields.size() < 2) {
        return false;
    }

    auto name = fields.at(1);

    return name == package_name;
#endif
}

}
