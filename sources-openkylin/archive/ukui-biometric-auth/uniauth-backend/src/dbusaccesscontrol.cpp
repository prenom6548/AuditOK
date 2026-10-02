/*
 * Copyright (C) 2026 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "dbusaccesscontrol.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStringList>
#include <QDateTime>
#include <QDebug>

namespace UniauthDBusAccess
{
#define EXTRA_WHITELIST_PATH QStringLiteral("/etc/biometric-auth/org.ukui.UniauthBackend.ini")

static QStringList fixedCallerWhitelist()
{
    return QStringList(
        { QStringLiteral("/usr/sbin/ukui-greeter"),
          QStringLiteral("/usr/bin/ukui-screensaver-backend"),
          QStringLiteral("/usr/bin/ukui-screensaver-dialog"),
          QStringLiteral("/usr/sbin/lightdm"),
          QStringLiteral("/usr/bin/ukui-control-center"),
          QStringLiteral("/usr/bin/ukui-session-tools"),
          QStringLiteral("/usr/bin/ukui-session"),
          QStringLiteral("/usr/lib/x86_64-linux-gnu/ukui-polkit/polkit-ukui-authentication-agent-1"),
          QStringLiteral("/usr/lib/aarch64-linux-gnu/ukui-polkit/polkit-ukui-authentication-agent-1"),
          QStringLiteral("/usr/lib/loongarch64-linux-gnu/ukui-polkit/polkit-ukui-authentication-agent-1"),
          QStringLiteral("/usr/lib/mips64el-linux-gnu/ukui-polkit/polkit-ukui-authentication-agent-1"),
          QStringLiteral("/usr/lib/sw_64-linux-gnu/ukui-polkit/polkit-ukui-authentication-agent-1"),
          QStringLiteral("/usr/lib/riscv64-linux-gnu/ukui-polkit/polkit-ukui-authentication-agent-1"),
          QStringLiteral("/usr/bin/bioauth"),
          QStringLiteral("/usr/bin/biometric-manager"),
          QStringLiteral("/usr/bin/ukui-biometric-manager"),
          QStringLiteral("/usr/sbin/box-manager"),
          QStringLiteral("/usr/share/kare/plugins/kare-polkit/polkit-kare-authentication-agent-1") });
}

/** 固定列表 + INI 中 [org.ukui.UniauthBackend] whitelist=（分号分隔） */
static QStringList fullCallerWhitelist()
{
    static QStringList cache;
    static qint64 lastMtime = -2;

    const QStringList base = fixedCallerWhitelist();
    const QFileInfo fi(EXTRA_WHITELIST_PATH);
    const qint64 mtime = fi.exists() ? fi.lastModified().toMSecsSinceEpoch() : -1;
    if (mtime == lastMtime && !cache.isEmpty()) {
        return cache;
    }
    lastMtime = mtime;

    QStringList list = base;
    if (fi.isReadable()) {
        QSettings settings(fi.absoluteFilePath(), QSettings::IniFormat);
        settings.beginGroup(QStringLiteral("org.ukui.UniauthBackend"));
        for (const QString &raw : settings.value(QStringLiteral("whitelist")).toStringList()) {
            const QString p = raw.trimmed();
            if (!p.isEmpty() && !list.contains(p)) {
                list.append(p);
            }
        }
        settings.endGroup();
    }
    cache = list;
    return cache;
}

int callerUid(const QDBusConnection &conn, const QDBusMessage &msg)
{
    QDBusConnectionInterface *intf = conn.interface();
    if (!intf) {
        return -1;
    }
    return intf->serviceUid(msg.service()).value();
}

QString callerExecutablePath(const QDBusConnection &conn, const QDBusMessage &msg)
{
    QDBusConnectionInterface *intf = conn.interface();
    if (!intf) {
        return QString();
    }
    const uint pid = intf->servicePid(msg.service()).value();
    if (pid == 0) {
        return QString();
    }
    // 使用 /proc/<pid>/exe 解析真实可执行文件路径
    const QString exeLink = QStringLiteral("/proc/%1/exe").arg(pid);
    QFileInfo fi(exeLink);
    if (!fi.exists()) {
        return QString();
    }
    QString exePath = fi.canonicalFilePath();
    if (exePath.isEmpty()) {
        exePath = fi.symLinkTarget();
    }
    if (exePath.isEmpty() || !exePath.startsWith(QLatin1Char('/'))) {
        return QString();
    }
    // 进程启动后若磁盘上的可执行文件被 unlink（如包升级替换），内核会在路径后附加 " (deleted)"，
    // 进程本身仍是原程序，剥离后缀后按正常路径参与白名单校验。
    const QString deletedSuffix = QStringLiteral(" (deleted)");
    qDebug() << "exePath:" << exePath;
    if (exePath.endsWith(deletedSuffix)) {
        exePath.chop(deletedSuffix.length());
    }
    return exePath;
}

bool isCallerRoot(const QDBusConnection &conn, const QDBusMessage &msg)
{
    return callerUid(conn, msg) == 0;
}

bool isCallerProcessWhitelisted(const QDBusConnection &conn, const QDBusMessage &msg)
{
    const QString exePath = callerExecutablePath(conn, msg);
    return fullCallerWhitelist().contains(exePath);
}

} // namespace UniauthDBusAccess
