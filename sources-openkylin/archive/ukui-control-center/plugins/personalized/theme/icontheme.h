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
#ifndef ICONTHEME_H
#define ICONTHEME_H
#include <QStringList>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <KSharedConfig>

class IconThemeDir;
typedef struct _theme_date
{
    QString example, screenshot;
    bool isHidden = false;
    bool isValid = false;
    KSharedConfig::Ptr sharedConfig;

    int defSize[6];
    QList<int> sizes[6];

    int depth;
    QString dir, themename, internalName;
    QStringList inherits;
    QVector<IconThemeDir *> dirs;
    QVector<IconThemeDir *> scaledDirs;
}themeDate;

enum Context {
    Any,
    Action,
    Application,
    Device,
    MimeType,
    FileSystem,
    Animation,
    Category,
    Emblem,
    Emote,
    International,
    Place,
    StatusIcon
};

enum Type {
    Fixed,
    Scalable,
    Threshold
};


class IconThemeDir
{
public:
    IconThemeDir(const QString &basedir, const QString &themedir, const KConfigGroup &config);

    bool isValid() const
    {
        return mbValid;
    }
    QString iconPath(const QString &name) const;
    QStringList iconList() const;
    QString constructFileName(const QString &file) const
    {
        return mBaseDir + mThemeDir + QLatin1Char('/') + file;
    }

    Context context() const
    {
        return mContext;
    }
    Type type() const
    {
        return mType;
    }
    int size() const
    {
        return mSize;
    }
    int scale() const
    {
        return mScale;
    }
    int minSize() const
    {
        return mMinSize;
    }
    int maxSize() const
    {
        return mMaxSize;
    }
    int threshold() const
    {
        return mThreshold;
    }

private:
    bool mbValid;
    Type mType;
    Context mContext;
    int mSize, mScale, mMinSize, mMaxSize;
    int mThreshold;

    const QString mBaseDir;
    const QString mThemeDir;
};

class IconTheme
{
public:
    IconTheme(const QString &name);
    ~IconTheme();

    static QStringList list();
    static QString defaultThemeName();
    QString iconPath(const QString &name, int size, qreal scale);

    bool isValid()
    {
        return mThemeDate.isValid;
    }

private:
    static QStringList iconThemeList;
    themeDate mThemeDate;
};

#endif // ICONTHEME_H
