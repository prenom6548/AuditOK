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

#ifndef UKUI_FILE_METADATA_BOOKMARK_H
#define UKUI_FILE_METADATA_BOOKMARK_H

#include "ukui-file-metadata_global.h"

#include <QHash>
#include <QList>
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>

namespace UkuiFileMetadata {

class FreedesktopApplicationInfoPrivate;

class UKUIFILEMETADATA_EXPORT FreedesktopApplicationInfo
{
public:
    FreedesktopApplicationInfo();
    FreedesktopApplicationInfo(const FreedesktopApplicationInfo &other);
    FreedesktopApplicationInfo &operator=(const FreedesktopApplicationInfo &other);
    FreedesktopApplicationInfo(FreedesktopApplicationInfo &&other) noexcept;
    FreedesktopApplicationInfo &operator=(FreedesktopApplicationInfo &&other) noexcept;
    ~FreedesktopApplicationInfo();

    QString href() const;
    QString name() const;
    QString exec() const;
    QString modified() const;
    int count() const;

    void setHref(const QString &value);
    void setName(const QString &value);
    void setExec(const QString &value);
    void setModified(const QString &value);
    void setCount(int value);

private:
    QSharedDataPointer<FreedesktopApplicationInfoPrivate> d;
};

class BookMarkPrivate;

class UKUIFILEMETADATA_EXPORT BookMark
{
public:
    BookMark();
    BookMark(const BookMark &other);
    BookMark &operator=(const BookMark &other);
    BookMark(BookMark &&other) noexcept;
    BookMark &operator=(BookMark &&other) noexcept;
    ~BookMark();

    QString id() const;
    QString added() const;
    QString modified() const;
    QString visited() const;

    QString href() const;
    QString title() const;
    QString description() const;
    QString showInToolbar() const;

    /**
     * @brief 获取 freedesktop `bookmark:icon` 的主题图标名称。
     * @return `name` 属性值；若不存在返回空字符串。
     */
    QString iconName() const;

    /**
     * @brief 获取 freedesktop `bookmark:icon` 的图标资源地址。
     * @return `href` 属性值；若不存在返回空字符串。
     */
    QString iconHref() const;

    /**
     * @brief 获取 freedesktop `bookmark:icon` 的图标 MIME 类型。
     * @return `type` 属性值；若不存在返回空字符串。
     */
    QString iconType() const;

    QString mimeType() const;

    /**
     * @brief 获取 freedesktop `bookmark:private` 标记。
     * @return `true` 表示书签被标记为 private，否则返回 `false`。
     */
    bool privateFlag() const;

    const QHash<QString, QStringList>& kdeMetaData() const;

    void setId(const QString &value);
    void setAdded(const QString &value);
    void setModified(const QString &value);
    void setVisited(const QString &value);

    void setHref(const QString &value);
    void setTitle(const QString &value);
    void setDescription(const QString &value);
    void setShowInToolbar(const QString &value);

    /**
     * @brief 设置 freedesktop `bookmark:icon` 的主题图标名称。
     * @param value `name` 属性值。
     */
    void setIconName(const QString &value);

    /**
     * @brief 设置 freedesktop `bookmark:icon` 的图标资源地址。
     * @param value `href` 属性值。
     */
    void setIconHref(const QString &value);

    /**
     * @brief 设置 freedesktop `bookmark:icon` 的图标 MIME 类型。
     * @param value `type` 属性值。
     */
    void setIconType(const QString &value);

    void setMimeType(const QString &value);

    /**
     * @brief 设置 freedesktop `bookmark:private` 标记。
     * @param value 是否标记为 private。
     */
    void setPrivateFlag(bool value);

    void addKdeMetaValue(const QString &key, const QString &value);

private:
    QSharedDataPointer<BookMarkPrivate> d;
};

class BookMarksGroupPrivate;

class UKUIFILEMETADATA_EXPORT BookMarksGroup
{
public:
    BookMarksGroup();
    BookMarksGroup(const BookMarksGroup &other);
    BookMarksGroup &operator=(const BookMarksGroup &other);
    BookMarksGroup(BookMarksGroup &&other) noexcept;
    BookMarksGroup &operator=(BookMarksGroup &&other) noexcept;
    ~BookMarksGroup();

    QString id() const;
    QString added() const;
    QString modified() const;
    QString visited() const;

    QString title() const;
    QString description() const;
    QString folded() const;
    QString toolbar() const;

    const QList<BookMark>& bookmarks() const;
    const QList<BookMarksGroup>& groups() const;

    const QHash<QString, QStringList>& kdeMetaData() const;

    void setId(const QString &value);
    void setAdded(const QString &value);
    void setModified(const QString &value);
    void setVisited(const QString &value);

    void setTitle(const QString &value);
    void setDescription(const QString &value);
    void setFolded(const QString &value);
    void setToolbar(const QString &value);

    void addBookmark(const BookMark &bookmark);
    void addGroup(const BookMarksGroup &group);

    void addKdeMetaValue(const QString &key, const QString &value);

private:
    QSharedDataPointer<BookMarksGroupPrivate> d;
};

} // namespace UkuiFileMetadata

#endif // UKUI_FILE_METADATA_BOOKMARK_H
