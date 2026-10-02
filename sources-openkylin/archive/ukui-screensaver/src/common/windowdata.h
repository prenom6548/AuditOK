// common/windowinfo.h
#ifndef WINDOWINFO_H
#define WINDOWINFO_H

#include <QString>
#include <QIcon>
#include <QMetaType>
#include <QDBusArgument>

/**
 * 窗口信息结构体
 */
namespace WindowInfo{
    struct WindowInfo {
        QString wid;    //窗口ID
        QString appId;  // 应用程序ID
        QString icon;   // 应用程序图标
        QString name;   //应用名称
    };
}
Q_DECLARE_METATYPE(WindowInfo::WindowInfo)

struct InhibitorInfo
{
    QString name;
    QString icon;
    int uid;
};
using InhibitorInfo = struct InhibitorInfo;

#endif // WINDOWINFO_H
