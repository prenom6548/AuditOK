#include <QCoreApplication>
#include "kappmanager.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    bool success = false;
    kdk::AppInfo *info = nullptr;
    QList<kdk::AppInfo *> list;

    kdk::KAppManager manager;

    // 获取系统中的应用程序列表
    qDebug() << "Application list:";
    list = manager.getApplications();
    for (kdk::AppInfo *info : list)
    {
        qDebug() << info->desktopPath();
        qDebug() << "\t" << info->name();
        qDebug() << "\t" << info->exec();
        qDebug() << "\t" << info->icon();
        delete info;
    }

    // 根据mime类型获取可用的应用程序
    qDebug() << "Application list By Mime:";
    list = manager.getApplicationByMime("x-scheme-handler/vscode");
    for (kdk::AppInfo *info : list)
    {
        qDebug() << info->desktopPath();
        qDebug() << "\t" << info->name();
        qDebug() << "\t" << info->exec();
        qDebug() << "\t" << info->icon();
        delete info;
    }

    // 获取系统中开机自启的应用程序列表
    qDebug() << "Auto start application list:";
    list = manager.getAutoStartApplications();
    for (kdk::AppInfo *info : list)
    {
        qDebug() << info->desktopPath();
        qDebug() << "\t" << info->name();
        qDebug() << "\t" << info->exec();
        qDebug() << "\t" << info->icon();
        delete info;
    }

    // 设置应用程序为开机自启
    success = manager.addAutoStartApplication("/usr/share/applications/atril.desktop");
    qDebug() << "Add auto start application " << (success ? "success" : "fail");

    // 删除开机自启应用
    success = manager.removeAutoStartApplication("/home/kylin/.config/autostart/atril.desktop");
    qDebug() << "Remove auto start application " << (success ? "success" : "fail");

    // 获取打开指定mime类型文件的默认应用程序
    info = manager.getDefaultApplicationByMime("x-scheme-handler/http");
    if (info)
    {
        qDebug() << info->desktopPath();
        delete info;
    }

    // 获取打开指定枚举类型文件的默认应用程序
    info = manager.getDefaultApplicationByType(kdk::KAppManager::VIDEO);
    if (info)
    {
        qDebug() << info->desktopPath();
        delete info;
    }

    // 设置指定mime类型的默认打开程序
    success = manager.setDefaultApplicationByMime("/usr/share/applications/code-url-handler.desktop", "x-scheme-handler/vscode");
    qDebug() << "Set default application by mime " << (success ? "success" : "fail");

    // 设置指定枚举类型的默认打开程序
    success = manager.setDefaultApplicationByType("/usr/share/applications/microsoft-edge.desktop", kdk::KAppManager::BROWSER);
    qDebug() << "Set default application by type " << (success ? "success" : "fail");

    return a.exec();
}
