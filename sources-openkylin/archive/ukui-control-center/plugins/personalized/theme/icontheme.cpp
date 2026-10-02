/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "icontheme.h"

#include <KConfigGroup>
#include <QDebug>
#include <cmath>

QStringList IconTheme::iconThemeList;

IconTheme::IconTheme(const QString &name)
{
    mThemeDate.internalName = name;

    QStringList themeDirs;
    QSet<QString> addedDirs;

    QStringList icondirs;

    icondirs.append(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("icons"), QStandardPaths::LocateDirectory));

    QString fileName, mainSection;
    for (QString it : icondirs) {
        qDebug() << Q_FUNC_INFO << "icon theme path" << it;
        QString dir = it + QLatin1Char('/') + name + QLatin1Char('/');
        if (QDir(dir).exists()) {
            themeDirs.append(dir);
            if (mThemeDate.dir.isEmpty()) {
                fileName = dir + QStringLiteral("index.theme");
                if (!QFile::exists(fileName)) {
                    qDebug() << Q_FUNC_INFO << "icon theme path index.theme not exist" << dir;
                    continue;
                }
                qDebug() << Q_FUNC_INFO << "icon theme path index.theme exist" << dir;
                mThemeDate.dir = dir;
                mainSection = QStringLiteral("Icon Theme");
            }
        }
    }

    if (mThemeDate.dir.isEmpty()) {
        qWarning() << "Icon theme : " << name << " not found.";
        return;
    }

    mThemeDate.sharedConfig = KSharedConfig::openConfig(fileName, KConfig::NoGlobals);

    KConfigGroup cfg(mThemeDate.sharedConfig, mainSection);
    mThemeDate.themename = cfg.readEntry("Name");
    mThemeDate.inherits = cfg.readEntry("Inherits", QStringList());
    if (name != defaultThemeName()) {
        for (QStringList::Iterator it = mThemeDate.inherits.begin(); it != mThemeDate.inherits.end(); ++it) {
            if (*it == QLatin1String("default")) {
                *it = defaultThemeName();
            }
        }

    }

    mThemeDate.isHidden = cfg.readEntry("Hidden", false);

    const QStringList dirs = cfg.readPathEntry("Directories", QStringList())
                           + cfg.readPathEntry("ScaledDirectories", QStringList());

    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
        KConfigGroup cg(mThemeDate.sharedConfig, *it);
        for (QStringList::ConstIterator itDir = themeDirs.constBegin(); itDir != themeDirs.constEnd(); ++itDir) {
            const QString currentDir(*itDir + *it + QLatin1Char('/'));
            if (!addedDirs.contains(currentDir) && QDir(currentDir).exists()) {
                addedDirs.insert(currentDir);
                IconThemeDir *dir = new IconThemeDir(*itDir, *it, cg);
                if (dir->isValid()) {
                    if (dir->scale() > 1) {
                        mThemeDate.scaledDirs.append(dir);
                    } else {
                        mThemeDate.dirs.append(dir);
                    }
                } else {
                    delete dir;
                }

            }
        }
    }
    mThemeDate.isValid = !mThemeDate.dirs.isEmpty() || !mThemeDate.scaledDirs.isEmpty();
}

IconTheme::~IconTheme()
{
    if (!mThemeDate.scaledDirs.isEmpty()) {
        qDeleteAll(mThemeDate.scaledDirs);
    }
    if (!mThemeDate.dirs.isEmpty()) {
        qDeleteAll(mThemeDate.dirs);
    }
}

QStringList IconTheme::list()
{
    if (!iconThemeList.isEmpty())
        return iconThemeList;

    QStringList icondirs;

    icondirs.append(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("icons"), QStandardPaths::LocateDirectory));

    for (QString it : icondirs) {
        QStringList themes = QDir(it).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (QString it_1 : themes) {
            if (it_1.startsWith(QLatin1String("default.")))
                continue;
            if (!QFileInfo::exists(it + QLatin1Char('/') + it_1 + QLatin1String("/index.theme")))
                continue;

            IconTheme oink(it_1);
            if (!oink.isValid())
                continue;
            if (!iconThemeList.contains(it_1))
                iconThemeList.append(it_1);
        }
    }
    return iconThemeList;
}

QString IconTheme::defaultThemeName()
{
    return QStringLiteral("hicolor");
}

QString IconTheme::iconPath(const QString &name, int size, qreal scale)
{
    QString path;
    QString tempPath;

    int integerScale = std::ceil(scale);

    for (IconThemeDir *dir : mThemeDate.scaledDirs) {
        if (dir->scale() != integerScale)
            continue;
        if ((dir->type() == Type::Fixed) && (dir->size() != size))
            continue;

        if ((dir->type() == Type::Scalable) && ((size < dir->minSize()) || (size > dir->maxSize())))
            continue;

        if ((dir->type() == Type::Threshold) && (abs(dir->size() - size) > dir->threshold()))
            continue;

        tempPath = dir->iconPath(name);

        if (tempPath.isEmpty()) {
            continue;
        }

        path = tempPath;
        break;
    }
    if (path.isEmpty()) {
        for (IconThemeDir *dir : mThemeDate.dirs) {
            if ((dir->type() == Type::Fixed) && (dir->size() != size))
                continue;

            if ((dir->type() == Type::Scalable) && ((size < dir->minSize()) || (size > dir->maxSize())))
                continue;

            if ((dir->type() == Type::Threshold) && (abs(dir->size() - size) > dir->threshold()))
                continue;

            tempPath = dir->iconPath(name);

            if (tempPath.isEmpty()) {
                continue;
            }

            path = tempPath;
            break;
        }
    }
    return path;
}

IconThemeDir::IconThemeDir(const QString &basedir, const QString &themedir, const KConfigGroup &config):
    mbValid(false),
    mType(Type::Fixed),
    mSize(config.readEntry("Size", 0)),
    mScale(config.readEntry("Scale", 1)),
    mMinSize(1),
    mMaxSize(50),
    mThreshold(2),
    mBaseDir(basedir),
    mThemeDir(themedir)
{
    if (mSize == 0) {
        return;
    }

    QString tmp = config.readEntry(QStringLiteral("Context"));
    if (tmp == QLatin1String("Devices")) {
        mContext = Context::Device;
    } else if (tmp == QLatin1String("MimeTypes")) {
        mContext = Context::MimeType;
    } else if (tmp == QLatin1String("FileSystems")) {
        mContext = Context::FileSystem;
    } else if (tmp == QLatin1String("Applications")) {
        mContext = Context::Application;
    } else if (tmp == QLatin1String("Actions")) {
        mContext = Context::Action;
    } else if (tmp == QLatin1String("Animations")) {
        mContext = Context::Animation;
    } else if (tmp == QLatin1String("Categories")) {
        mContext = Context::Category;
    } else if (tmp == QLatin1String("Emblems")) {
        mContext = Context::Emblem;
    } else if (tmp == QLatin1String("Emotes")) {
        mContext = Context::Emote;
    } else if (tmp == QLatin1String("International")) {
        mContext = Context::International;
    } else if (tmp == QLatin1String("Places")) {
        mContext = Context::Place;
    } else if (tmp == QLatin1String("Status")) {
        mContext = Context::StatusIcon;
    } else if (tmp == QLatin1String("Stock")) {
        return;
    } else if (tmp.isEmpty()) {
    } else {
//        qWarning() << "Invalid Context=" << tmp << "line for icon theme: " << constructFileName(QString());
        return;
    }
    tmp = config.readEntry(QStringLiteral("Type"), QStringLiteral("Threshold"));
    if (tmp == QLatin1String("Fixed")) {
        mType = Type::Fixed;
    } else if (tmp == QLatin1String("Scalable")) {
        mType = Type::Scalable;
    } else if (tmp == QLatin1String("Threshold")) {
        mType = Type::Threshold;
    } else {
//        qWarning() << "Invalid Type=" << tmp << "line for icon theme: " << constructFileName(QString());
        return;
    }
    if (mType == Type::Scalable) {
        mMinSize = config.readEntry(QStringLiteral("MinSize"), mSize);
        mMaxSize = config.readEntry(QStringLiteral("MaxSize"), mSize);
    } else if (mType == Type::Threshold) {
        mThreshold = config.readEntry(QStringLiteral("Threshold"), 2);
    }
    mbValid = true;
}

QString IconThemeDir::iconPath(const QString &name) const
{
    if (!mbValid)
        return QString();

    const QString file = mBaseDir + mThemeDir + QLatin1Char('/') + name;
    return QFileInfo::exists(file) ? file : QString();
}
