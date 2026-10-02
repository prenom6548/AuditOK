/*
 * libkysdk-appmanager's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "appinfo.h"
#include "appinfo_p.h"
#include <QDir>
#include <QSettings>
#include <QLocale>
#include <memory>

kdk::AppInfo::AppInfo(const QString &desktopFilePath, QObject *parent)
    : QObject(parent),
      d_ptr(new AppInfoPrivate(desktopFilePath))
{
    Q_D(AppInfo);
    d->init(desktopFilePath, this);
}

kdk::AppInfo::AppInfo(const kdk::AppInfo &other)
    : QObject(other.parent()),
      d_ptr(new AppInfoPrivate(*other.d_func()))
{

}

kdk::AppInfo::~AppInfo()
{
}

QString kdk::AppInfo::name() const
{
    Q_D(const AppInfo);
    return d->mName;
}

QString kdk::AppInfo::type() const
{
    Q_D(const AppInfo);
    return d->mType;
}

QString kdk::AppInfo::exec() const
{
    Q_D(const AppInfo);
    return d->mExec;
}

QString kdk::AppInfo::icon() const
{
    Q_D(const AppInfo);
    return d->mIcon;
}

QString kdk::AppInfo::comment() const
{
    Q_D(const AppInfo);
    return d->mComment;
}

QString kdk::AppInfo::desktopName() const
{
    Q_D(const AppInfo);
    return d->mDesktopEntryName;
}

QString kdk::AppInfo::desktopPath() const
{
    Q_D(const AppInfo);
    return d->mDesktopEntryPath;
}

QString kdk::AppInfo::genericName() const
{
    Q_D(const AppInfo);
    return d->mGenericName;
}

QString kdk::AppInfo::terminalOptions() const
{
    Q_D(const AppInfo);
    return d->mTerminalOptions;
}

QStringList kdk::AppInfo::mimeTypes() const
{
    Q_D(const AppInfo);
    return d->mMimeTypes;
}

bool kdk::AppInfo::terminal() const
{
    Q_D(const AppInfo);
    return d->mTerminal;
}

bool kdk::AppInfo::isVaild() const
{
    Q_D(const AppInfo);
    return d->mVaild;
}

QVariant kdk::AppInfo::property(const QString &name) const
{
    Q_D(const AppInfo);

    if (name == QLatin1String("Type")) {
        return d->mType;
    } else if (name == QLatin1String("Name")) {
        return d->mName;
    } else if (name == QLatin1String("Exec")) {
        return d->mExec;
    } else if (name == QLatin1String("Icon")) {
        return d->mIcon;
    } else if (name == QLatin1String("TerminalOptions")) {
        return d->mTerminalOptions;
    } else if (name == QLatin1String("Path")) {
        return d->mWorkingDirectory;
    } else if (name == QLatin1String("Comment")) {
        return d->mComment;
    } else if (name == QLatin1String("GenericName")) {
        return d->mGenericName;
    } else if (name == QLatin1String("DesktopEntryPath")) {
        return d->mDesktopEntryPath;
    } else if (name == QLatin1String("DesktopEntryName")) {
        return d->mDesktopEntryName;
    } else if (name == QLatin1String("UntranslatedName")) {
        return d->mUntranslatedName;
    } else if (name == QLatin1String("UntranslatedGenericName")) {
        return d->mUntranslatedGenericName;
    }

    auto it = d->mMapProps.constFind(name);

    if (it != d->mMapProps.cend()) {
        return it.value().toString();
    }

    return QString();
}

kdk::AppInfoPrivate::AppInfoPrivate(const QString &path)
    : mVaild(true)
{
}

kdk::AppInfoPrivate::~AppInfoPrivate()
{

}

void kdk::AppInfoPrivate::init(const QString &desktop, AppInfo *q)
{
    mDesktopEntryPath = desktop;
    
    if (!QFile::exists(desktop))
    {
        mVaild = false;
        return;
    }

    QSettings setting(desktop, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup("Desktop Entry");

    bool hidden = setting.value("Hidden", false).toBool();
    if (hidden)
    {
        mVaild = false;
    }

    // Path To desktopEntryName
    // (e.g. "/usr/share/applications/atril.desktop" --> "atril")
    QString name = desktop;
    int i = name.lastIndexOf('/');
    name = name.mid(i + 1);
    i = name.lastIndexOf('.');
    if (i != -1)
        name.truncate(i);

    QLocale systemLocale = QLocale::system();
    QString lang = QString("[%1]").arg(systemLocale.name());
    // 应该是根据环境变量确定地区，这里直接
    mName = setting.value("Name" + lang, "").toString().toUtf8();
    if (mName.isEmpty())
    {
        mName = name;
    }

    mType = setting.value("Type", "").toString();
    if (mType.isEmpty())
    {
        mType = "Application";
    }
    else if (mType != "Application" && mType != "Service")
    {
        mVaild = false;
    }

    mExec = setting.value("Exec", "").toString();

    mIcon = setting.value("Icon", "").toString();
    mTerminal = setting.value("Terminal", true).toBool();
    mTerminalOptions = setting.value("TerminalOptions", "").toString();
    mWorkingDirectory = setting.value("Path", "").toString();
    mComment = setting.value("Comment", "").toString();
    mGenericName = setting.value("GenericName" + lang, "").toString();

    mUntranslatedGenericName = setting.value("GenericName", "").toString();
    mUntranslatedName = setting.value("Name", "").toString();

    mDesktopEntryName = name;

    if (setting.contains("Categories"))
    {
        mCategories = setting.value("Categories").toString().split(";");
    }

    if (setting.contains("MimeType"))
    {
        QStringList mimeTypes = setting.value("MimeType").toString().split(";");
        for(QString &mimeType : mimeTypes)
        {
            mMimeTypes.append(mimeType);
        }
    }

    for (QString key : setting.allKeys())
    {
        if (!key.contains(QLatin1Char('[')))
        {
            // qCDebug(SERVICES) << "  Key =" << key << " Data =" << it.value();
            if (key == QLatin1String("X-Flatpak-RenamedFrom"))
            {
                mMapProps.insert(key, setting.value(key).toString().split(";"));
            }
            else
            {
                mMapProps.insert(key, setting.value(key));
            }
        }
    }

    setting.endGroup();
}
