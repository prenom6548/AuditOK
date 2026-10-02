/*
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "bookmark.h"

#include <QSharedData>

namespace UkuiFileMetadata {

class FreedesktopApplicationInfoPrivate : public QSharedData
{
public:
    QString href;
    QString name;
    QString exec;
    QString modified;
    int count = -1;
};

FreedesktopApplicationInfo::FreedesktopApplicationInfo()
    : d(new FreedesktopApplicationInfoPrivate)
{
}

FreedesktopApplicationInfo::FreedesktopApplicationInfo(const FreedesktopApplicationInfo &other) = default;

FreedesktopApplicationInfo &FreedesktopApplicationInfo::operator=(const FreedesktopApplicationInfo &other) = default;

FreedesktopApplicationInfo::FreedesktopApplicationInfo(FreedesktopApplicationInfo &&other) noexcept = default;

FreedesktopApplicationInfo &FreedesktopApplicationInfo::operator=(FreedesktopApplicationInfo &&other) noexcept = default;

FreedesktopApplicationInfo::~FreedesktopApplicationInfo() = default;

QString FreedesktopApplicationInfo::href() const
{
    return d->href;
}

QString FreedesktopApplicationInfo::name() const
{
    return d->name;
}

QString FreedesktopApplicationInfo::exec() const
{
    return d->exec;
}

QString FreedesktopApplicationInfo::modified() const
{
    return d->modified;
}

int FreedesktopApplicationInfo::count() const
{
    return d->count;
}

void FreedesktopApplicationInfo::setHref(const QString &value)
{
    d->href = value;
}

void FreedesktopApplicationInfo::setName(const QString &value)
{
    d->name = value;
}

void FreedesktopApplicationInfo::setExec(const QString &value)
{
    d->exec = value;
}

void FreedesktopApplicationInfo::setModified(const QString &value)
{
    d->modified = value;
}

void FreedesktopApplicationInfo::setCount(int value)
{
    d->count = value;
}

class BookMarkPrivate : public QSharedData
{
public:
    QString id;
    QString added;
    QString modified;
    QString visited;
    QString href;
    QString title;
    QString description;
    QString showInToolbar;
    QString iconName;
    QString iconHref;
    QString iconType;
    QString mimeType;
    bool privateFlag = false;
    QHash<QString, QStringList> kdeMetaData;
};

BookMark::BookMark()
    : d(new BookMarkPrivate)
{
}

BookMark::BookMark(const BookMark &other) = default;

BookMark &BookMark::operator=(const BookMark &other) = default;

BookMark::BookMark(BookMark &&other) noexcept = default;

BookMark &BookMark::operator=(BookMark &&other) noexcept = default;

BookMark::~BookMark() = default;

QString BookMark::id() const
{
    return d->id;
}

QString BookMark::added() const
{
    return d->added;
}

QString BookMark::modified() const
{
    return d->modified;
}

QString BookMark::visited() const
{
    return d->visited;
}

QString BookMark::href() const
{
    return d->href;
}

QString BookMark::title() const
{
    return d->title;
}

QString BookMark::description() const
{
    return d->description;
}

QString BookMark::showInToolbar() const
{
    return d->showInToolbar;
}

QString BookMark::iconName() const
{
    return d->iconName;
}

QString BookMark::iconHref() const
{
    return d->iconHref;
}

QString BookMark::iconType() const
{
    return d->iconType;
}

QString BookMark::mimeType() const
{
    return d->mimeType;
}

bool BookMark::privateFlag() const
{
    return d->privateFlag;
}

const QHash<QString, QStringList>& BookMark::kdeMetaData() const
{
    return d->kdeMetaData;
}

void BookMark::setId(const QString &value)
{
    d->id = value;
}

void BookMark::setAdded(const QString &value)
{
    d->added = value;
}

void BookMark::setModified(const QString &value)
{
    d->modified = value;
}

void BookMark::setVisited(const QString &value)
{
    d->visited = value;
}

void BookMark::setHref(const QString &value)
{
    d->href = value;
}

void BookMark::setTitle(const QString &value)
{
    d->title = value;
}

void BookMark::setDescription(const QString &value)
{
    d->description = value;
}

void BookMark::setShowInToolbar(const QString &value)
{
    d->showInToolbar = value;
}

void BookMark::setIconName(const QString &value)
{
    d->iconName = value;
}

void BookMark::setIconHref(const QString &value)
{
    d->iconHref = value;
}

void BookMark::setIconType(const QString &value)
{
    d->iconType = value;
}

void BookMark::setMimeType(const QString &value)
{
    d->mimeType = value;
}

void BookMark::setPrivateFlag(bool value)
{
    d->privateFlag = value;
}

void BookMark::addKdeMetaValue(const QString &key, const QString &value)
{
    if (key.isEmpty() || value.isEmpty()) {
        return;
    }
    d->kdeMetaData[key].append(value);
}

class BookMarksGroupPrivate : public QSharedData
{
public:
    QString id;
    QString added;
    QString modified;
    QString visited;
    QString title;
    QString description;
    QString folded;
    QString toolbar;
    QList<BookMark> bookmarks;
    QList<BookMarksGroup> groups;
    QHash<QString, QStringList> kdeMetaData;
};

BookMarksGroup::BookMarksGroup()
    : d(new BookMarksGroupPrivate)
{
}

BookMarksGroup::BookMarksGroup(const BookMarksGroup &other) = default;

BookMarksGroup &BookMarksGroup::operator=(const BookMarksGroup &other) = default;

BookMarksGroup::BookMarksGroup(BookMarksGroup &&other) noexcept = default;

BookMarksGroup &BookMarksGroup::operator=(BookMarksGroup &&other) noexcept = default;

BookMarksGroup::~BookMarksGroup() = default;

QString BookMarksGroup::id() const
{
    return d->id;
}

QString BookMarksGroup::added() const
{
    return d->added;
}

QString BookMarksGroup::modified() const
{
    return d->modified;
}

QString BookMarksGroup::visited() const
{
    return d->visited;
}

QString BookMarksGroup::title() const
{
    return d->title;
}

QString BookMarksGroup::description() const
{
    return d->description;
}

QString BookMarksGroup::folded() const
{
    return d->folded;
}

QString BookMarksGroup::toolbar() const
{
    return d->toolbar;
}

const QList<BookMark>& BookMarksGroup::bookmarks() const
{
    return d->bookmarks;
}

const QList<BookMarksGroup>& BookMarksGroup::groups() const
{
    return d->groups;
}

const QHash<QString, QStringList>& BookMarksGroup::kdeMetaData() const
{
    return d->kdeMetaData;
}

void BookMarksGroup::setId(const QString &value)
{
    d->id = value;
}

void BookMarksGroup::setAdded(const QString &value)
{
    d->added = value;
}

void BookMarksGroup::setModified(const QString &value)
{
    d->modified = value;
}

void BookMarksGroup::setVisited(const QString &value)
{
    d->visited = value;
}

void BookMarksGroup::setTitle(const QString &value)
{
    d->title = value;
}

void BookMarksGroup::setDescription(const QString &value)
{
    d->description = value;
}

void BookMarksGroup::setFolded(const QString &value)
{
    d->folded = value;
}

void BookMarksGroup::setToolbar(const QString &value)
{
    d->toolbar = value;
}

void BookMarksGroup::addBookmark(const BookMark &bookmark)
{
    d->bookmarks.append(bookmark);
}

void BookMarksGroup::addGroup(const BookMarksGroup &group)
{
    d->groups.append(group);
}

void BookMarksGroup::addKdeMetaValue(const QString &key, const QString &value)
{
    if (key.isEmpty() || value.isEmpty()) {
        return;
    }
    d->kdeMetaData[key].append(value);
}

} // namespace UkuiFileMetadata
