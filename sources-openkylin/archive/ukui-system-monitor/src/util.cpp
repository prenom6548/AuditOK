/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util.h"

#include <QIcon>
#include <QDirIterator>
#include <QDebug>
#include <fstream>
#include <sstream>
#include <QProcess>
#include <kysdk/kysdk-base/kyutils.h>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDebug>

std::string make_string(char *c_str)
{
    if (!c_str) {
        return string();
    }
    string s(c_str);
    g_free(c_str);
    return s;
}

static inline unsigned divide(unsigned *q, unsigned *r, unsigned d)
{
    *q = *r / d;
    *r = *r % d;
    return *q != 0;
}

QString getElidedText(QFont font, QString str, int MaxWidth)
{
    if (str.isEmpty())
    {
        return "";
    }

    QFontMetrics fontWidth(font);

    //计算字符串宽度
    //calculat the width of the string
    int width = fontWidth.width(str);

    //当字符串宽度大于最大宽度时进行转换
    //Convert when string width is greater than maximum width
    if (width >= MaxWidth)
    {
        //右部显示省略号
        //show by ellipsis in right
        str = fontWidth.elidedText(str, Qt::ElideRight, MaxWidth);
    }
    //返回处理后的字符串
    //return the string that is been handled
    return str;
}

QString getMiddleElidedText(QFont font, QString str, int MaxWidth)
{
    if (str.isEmpty())
    {
        return "";
    }

    QFontMetrics fontWidth(font);

    //计算字符串宽度
    int width = fontWidth.width(str);

    //当字符串宽度大于最大宽度时进行转换
    if (width >= MaxWidth)
    {
        //右部显示省略号
        str = fontWidth.elidedText(str, Qt::ElideMiddle, MaxWidth);
    }
    //返回处理后的字符串
    return str;
}

QString formatDurationForDisplay(unsigned centiseconds)
{
    unsigned weeks = 0, days = 0, hours = 0, minutes = 0, seconds = 0;

    (void)(divide(&seconds, &centiseconds, 100)
           && divide(&minutes, &seconds, 60)
           && divide(&hours, &minutes, 60)
           && divide(&days, &hours, 24)
           && divide(&weeks, &days, 7));

    QString formatTime;
    gchar *duration = NULL;

    if (weeks) {
        duration = g_strdup_printf("%uw%ud", weeks, days);
        formatTime = QString(QLatin1String(duration));
        if (duration) {
            g_free(duration);
            duration = NULL;
        }
        return formatTime;
    }

    if (days) {
        duration = g_strdup_printf("%ud%02uh", days, hours);
        formatTime = QString(QLatin1String(duration));
        if (duration) {
            g_free(duration);
            duration = NULL;
        }
        return formatTime;
    }

    if (hours) {
        duration = g_strdup_printf("%u:%02u:%02u", hours, minutes, seconds);
        formatTime = QString(QLatin1String(duration));
        if (duration) {
            g_free(duration);
            duration = NULL;
        }
        return formatTime;
    }

    duration = g_strdup_printf("%u:%02u.%02u", minutes, seconds, centiseconds);
    formatTime = QString(QLatin1String(duration));
    if (duration) {
        g_free(duration);
        duration = NULL;
    }
    return formatTime;
}

QString formatProcessState(QString state)
{
    QString status;
    if (state == "R") {
        status = QString(QObject::tr("Running"));//运行中
    }
    if (state == "T") {
        status = QString(QObject::tr("Stopped"));//已停止
    }
    if (state == "Z") {
        status = QString(QObject::tr("Zombie"));//僵死
    }
    if (state == "D") {
        status = QString(QObject::tr("Uninterruptible"));//不可中断
    }
    if (state == "S") {
        status = QString(QObject::tr("Sleeping"));//睡眠中
    }

    return status;
}

QString getNiceLevel(int nice)
{
    if (nice < -7)
        return QObject::tr("Very High");
    else if (nice < -2)
        return QObject::tr("High");
    else if (nice < 3)
        return QObject::tr("Normal");
    else if (nice < 7)
        return QObject::tr("Low");
    else
        return QObject::tr("Very Low");
}

QString getNiceLevelWithPriority(int nice)
{
    if (nice < -7)
        return QObject::tr("Very High Priority");
    else if (nice < -2)
        return QObject::tr("High Priority");
    else if (nice < 3)
        return QObject::tr("Normal Priority");
    else if (nice < 7)
        return QObject::tr("Low Priority");
    else
        return QObject::tr("Very Low Priority");
}

QString formatUnitSize(double v, const char** orders, int nb_orders)
{
    int order = 0;
    guint64 m_size = guint64(v);
    char data[128] = {0};
    snprintf(data,sizeof(data), "%lu",m_size);
    char result_data[128] = {0};
    double m_speed = 0;
    if (kdkVolumeBaseCharacterConvert(data, KDK_EXABYTE, result_data) == KDK_NOERR) {
        QString result_data_str = QString(result_data);
        result_data_str.replace("KB", "K");
        result_data_str.replace("MB", "M");
        result_data_str.replace("GB", "G");
        result_data_str.replace("TB", "T");
        result_data_str.replace("PB", "P");
        result_data_str.replace("EB", "E");

        result_data_str.replace("B", " B");
        result_data_str.replace("K", " KB");
        result_data_str.replace("M", " MB");
        result_data_str.replace("G", " GB");
        result_data_str.replace("T", " TB");
        result_data_str.replace("P", " PB");
        result_data_str.replace("E", " EB");
        sscanf(result_data, "%lf", &m_speed);
        return result_data_str;
    } else {
        while (v >= 1024 && order + 1 < nb_orders) {
            order++;
            v  = v/1024;
        }
        char buffer1[30] = {0};
        snprintf(buffer1, sizeof(buffer1), "%.1lf %s", v, orders[order]);
        return QString(buffer1);
    }
}

QString formatByteCount(double v)
{
    static const char* orders[] = { "B", "KB", "MB", "GB", "TB" };
    return formatUnitSize(v, orders, sizeof(orders)/sizeof(orders[0]));
}

QString getDeviceMountedPointPath(const QString &line)
{
    const QStringList items = line.split(" ");
    if (items.length() > 4) {
        return items.at(1);
    }
    else {
        return "";
    }
}

QString getFileContent(const QString &filePath)
{
    QFile fd(filePath);
    QString fileContent = "";
    if (fd.open(QFile::ReadOnly)) {
        fileContent = QLatin1String(fd.readAll());
        fd.close();
    }
    return fileContent;
}

QSet<QString> getFileContentsLineByLine(const QString &filePath)
{
    QString fileContent = getFileContent(filePath);
    return QSet<QString>::fromList(fileContent.split("\n"));
}

// 获取应用程序版本
QString getUsmVersion()
{
    QString versionText;
    QProcess proc;
    QStringList options;
    options << "-l" << "|" << "grep" << "ukui-system-monitor";
    proc.start("dpkg", options);
    proc.waitForFinished();
    QString dpkgInfo = proc.readAll();
    QStringList infoList = dpkgInfo.split("\n");
    for (int n = 0; n < infoList.size(); n++) {
        QString strInfoLine = infoList[n];
        if (strInfoLine.contains("ukui-system-monitor")) {
            QStringList lineInfoList = strInfoLine.split(QRegExp("[\\s]+"));
            if (lineInfoList.size() >= 3) {
                versionText = lineInfoList[2];
            }
            break;
        }
    }
    return versionText;
}

bool checkEfficiencyMode()
{
    const QString service = "com.kylin.ProcessManagerDaemon";
    const QString path = "/com/kylin/ProcessManagerDaemon/EfficiencyMode";

    // 检查对象路径是否存在（使用 Introspectable 接口）
    QDBusInterface introspect(service, path, "org.freedesktop.DBus.Introspectable",
                               QDBusConnection::systemBus());

    QDBusReply<QString> xmlReply = introspect.call("Introspect");
    if (!xmlReply.isValid()) {
        // 错误类型判断
        QDBusError err = xmlReply.error();
        if (err.type() == QDBusError::UnknownObject) {
            qDebug() << "✗ 对象路径不存在:" << path;
        } else {
            qDebug() << "✗ 路径访问错误:" << err.message();
        }
        return false;
    }
    qDebug() << "✓ 对象路径存在:" << path;

    return true;
}

bool checkMethodExist(QString service, QString path, QString method)
{
    // 当前国测版本不进运行环境需求，直接返回false
//    return false;
    // 1. 检查对象路径是否存在（使用 Introspectable 接口）
    QDBusInterface introspect(service, path, "org.freedesktop.DBus.Introspectable",
                               QDBusConnection::systemBus());

    QDBusReply<QString> xmlReply = introspect.call("Introspect");
    if (!xmlReply.isValid()) {
        QDBusError err = xmlReply.error();
        if (err.type() == QDBusError::UnknownObject) {
            qDebug() << "✗ 对象路径不存在:" << path;
        } else {
            qDebug() << "✗ 路径访问错误:" << err.message();
        }
        return false;
    }
    qDebug() << "✓ 对象路径存在:" << path;

    // 2. 解析 XML，查找 getProcRuntimeEnv 接口
    QString xmlContent = xmlReply.value();

    // 方法1：简单字符串查找（快速但不够严谨）
    if (xmlContent.contains(method)) {
        qDebug() << "✓ getProcRuntimeEnv 接口存在";
        return true;
    }

    qDebug() << "✗ getProcRuntimeEnv 接口不存在";
    return false;
}


