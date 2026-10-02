/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "utils.h"
#include "definetypes.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <gio/gio.h>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QMimeDatabase>
#include <QTextStream>
#include <QtX11Extras/QX11Info>
#include <X11/XKBlib.h>
#include <QSettings>

#define DRM_DIR "/sys/class/leds/"

void checkIslivecd()
{
    char cmd[128] = { 0 };
    char str[1024];
    FILE *fp = NULL;

    int n = sprintf(cmd, "cat /proc/cmdline");
    Q_UNUSED(n)

    fp = popen(cmd, "r");
    while (fgets(str, sizeof(str) - 1, fp)) {
        if (strstr(str, "boot=casper")) {
            printf("is livecd\n");
            exit(0);
        }
    }
    pclose(fp);

    QString filepath = QDir::homePath() + "/Desktop" + "/kylin-os-installer.desktop";
    QFileInfo file(filepath);
    if (!file.exists())
        return;
    if (getuid() != 999)
        return;
    exit(0);
}

void KillFocusOfKydroid()
{
    if (!isGreeterMode()) {
        QString username = getenv("USER");
        int uid = getuid();
        QDBusInterface *interface = new QDBusInterface(
            "cn.kylinos.Kydroid2", "/cn/kylinos/Kydroid2", "cn.kylinos.Kydroid2", QDBusConnection::systemBus());
        if (interface) {
            QDBusPendingCall pendingCall
                = interface->asyncCall(QStringLiteral("SetPropOfContainer"), username, uid, "is_kydroid_on_focus", "0");
            Q_UNUSED(pendingCall);
            interface->deleteLater();
            interface = nullptr;
        }
    }
}

/**
 * @brief 判断大写键状态
 * @return true: 大写锁定
 */
bool checkCapsLockState()
{
    QDir ledDir(DRM_DIR);
    QStringList leds = ledDir.entryList(QDir::Dirs);
    QString capsFile;

    for (int i = 0; i < leds.count(); i++) {
        if (leds.at(i).contains("capslock"))
            capsFile = leds.at(i);
    }
    QFile drmStatusFile(DRM_DIR + capsFile + "/brightness");
    if (drmStatusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&drmStatusFile);
        QString status = in.readLine();

        if (status == "0") {
            return false;
        } else {
            return true;
        }
    }

    bool capsState = false;
    unsigned int n;
    XkbGetIndicatorState(QX11Info::display(), XkbUseCoreKbd, &n);
    capsState = (n & 0x01) == 1;

    return capsState;
}

bool ispicture(QString filepath)
{
    QFileInfo file(filepath);
    if (file.exists() == false)
        return false;

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filepath);
    return mime.name().startsWith("image/");
}

double getDefaultFontSize()
{
    GSettingsSchemaSource *schema_source = NULL;
    GSettingsSchema *schema = NULL;
    double defaultFontSize = DEFAULT_FONT_SIZE;

    schema_source = g_settings_schema_source_get_default();
    if (schema_source) {
        schema = g_settings_schema_source_lookup(schema_source, GSETTINGS_SCHEMA_STYLE, TRUE);
        if (schema) {
            GSettings *gs = g_settings_new(GSETTINGS_SCHEMA_STYLE);
            if (gs) {
                GVariant *size = g_settings_get_default_value(gs, GLIB_KEY_SYSTEM_FONT_SIZE);
                QString fontsize(g_variant_get_string(size, NULL));
                g_variant_unref(size);
                g_object_unref(gs);
                defaultFontSize = fontsize.toDouble();
            }
            g_settings_schema_unref(schema);
        }
    }
    return defaultFontSize;
}

double getUserFontSize(QString userName)
{
    getDefaultFontSize();
    QString configPath;
    configPath = QString("/var/lib/lightdm-data/%1/ukui-greeter.conf").arg(userName);
    QFile configFile(configPath);
    double fontSize;
    if (configFile.exists()) {
        QSettings settings(configPath, QSettings::IniFormat);
        settings.beginGroup("Greeter");
        fontSize = settings.value("fontSize").toDouble();
    } else {
        fontSize = getDefaultFontSize();
    }
    return fontSize - getDefaultFontSize();
}

QString getUserThemeColor(QString userName)
{
    QString configPath;
    configPath = QString("/var/lib/lightdm-data/%1/ukui-greeter.conf").arg(userName);
    QFile configFile(configPath);
    QString themeColor = "daybreakBlue";
    if (configFile.exists()) {
        QSettings settings(configPath, QSettings::IniFormat);
        settings.beginGroup("Greeter");
        themeColor = settings.value("themeColor").toString();
    }
    return themeColor;
}
