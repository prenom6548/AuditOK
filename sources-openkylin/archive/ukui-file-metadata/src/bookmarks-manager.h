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

#ifndef UKUI_FILE_METADATA_BOOKMARKS_MANAGER_H
#define UKUI_FILE_METADATA_BOOKMARKS_MANAGER_H

#include "bookmark.h"
#include "ukui-file-metadata_global.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

namespace UkuiFileMetadata {

class BookMarksManagerPrivate;

/**
 * @brief XBEL 书签管理器。
 *
 * 负责解析 XBEL 文件并提供统一访问接口，包含：
 * 1. 书签树结构（根节点、分组、书签基本字段）；
 * 2. 可用于全文索引的文本行输出；
 * 3. 自定义 metadata 与扩展属性提取结果；
 * 4. freedesktop recently-used 相关扩展字段（groups / applications）。
 *
 * 该类可直接用于安装后的公共 API，具体解析实现通过 private 类封装。
 */
class UKUIFILEMETADATA_EXPORT BookMarksManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param filePath XBEL 文件路径。非空时会立即尝试解析。
     */
    explicit BookMarksManager(const QString &filePath = QString(), QObject *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~BookMarksManager() override;

    /**
     * @brief 当前解析结果是否有效。
     * @return `true` 表示文件可读且解析成功，`false` 表示失败或未设置有效文件。
     */
    bool isValid() const;

    /**
     * @brief 获取当前文件路径。
     * @return 当前设置的 XBEL 文件路径。
     */
    QString filePath() const;

    /**
     * @brief 设置并重新解析文件路径。
     * @param filePath 新的 XBEL 文件路径。与当前路径相同则不触发重新解析。
     */
    void setFilePath(const QString &filePath);

    /**
     * @brief 将当前解析文档保存回当前文件路径。
     * @return `true` 表示保存成功，`false` 表示保存失败。
     */
    bool save();

    /**
     * @brief 将当前解析文档另存到指定文件路径。
     * @param filePath 目标文件路径。
     * @return `true` 表示保存成功，`false` 表示保存失败。
     */
    bool saveAs(const QString &filePath);

    /**
     * @brief 获取最近一次解析错误信息。
     * @return 解析失败时的错误文本；当前结果有效或无错误时返回空字符串。
     */
    QString lastError() const;

    /**
     * @brief 获取最近一次 XML 解析错误的行号。
     * @return 若无可用行号返回 -1。
     */
    int errorLine() const;

    /**
     * @brief 获取最近一次 XML 解析错误的列号。
     * @return 若无可用列号返回 -1。
     */
    int errorColumn() const;

    /**
     * @brief 获取根节点标题。
     * @return `<xbel><title>` 内容，若不存在返回空字符串。
     */
    QString rootTitle() const;

    /**
     * @brief 获取根节点 dbusName。
     * @return 根节点 `dbusName` 属性值，若不存在返回空字符串。
     */
    QString dbusName() const;

    /**
     * @brief 获取解析后的根分组对象。
     * @return 根分组副本。
     */
    BookMarksGroup rootGroup() const;

    /**
     * @brief 获取用于全文索引/文本提取的聚合行。
     *
     * 包含基础字段、自定义 metadata、扩展属性等可检索文本。
     *
     * @return 文本行列表。
     */
    QStringList textLines() const;

    /**
     * @brief 获取自定义 metadata 解析结果。
     *
     * 格式示例：`metadata[owner].path: value`、
     * `metadata[owner].path@attr: value`、
     * `metadata[owner][href].path: value`。
     *
     * @return 自定义 metadata 文本行列表。
     */
    QStringList customMetaLines() const;

    /**
     * @brief 获取扩展属性解析结果。
     *
     * 格式示例：`attribute[element].attr: value`。
     *
     * @return 扩展属性文本行列表。
     */
    QStringList extendedAttributeLines() const;

    /**
     * @brief 获取全部 freedesktop 分组信息（按 href 聚合）。
     * @return `href -> group 列表` 的映射。
     */
    QHash<QString, QStringList> freedesktopGroups() const;

    /**
     * @brief 获取指定 href 的 freedesktop 分组。
     * @param href 书签 href。
     * @return 指定 href 的 group 列表；若不存在返回空列表。
     */
    QStringList freedesktopGroups(const QString &href) const;

    /**
     * @brief 获取全部 freedesktop 应用信息。
     * @return `FreedesktopApplicationInfo` 列表。
     */
    QList<FreedesktopApplicationInfo> freedesktopApplications() const;

    /**
     * @brief 获取指定 href 的 freedesktop 应用信息。
     * @param href 书签 href。
     * @return 指定 href 对应的应用信息列表；若不存在返回空列表。
     */
    QList<FreedesktopApplicationInfo> freedesktopApplications(const QString &href) const;

Q_SIGNALS:
    /**
     * @brief 解析内容已更新（包括文件监听触发的重解析）。
     */
    void changed();

    /**
     * @brief 解析或保存失败。
     * @param message 错误信息。
     */
    void errorOccurred(const QString &message);

private:
    Q_DISABLE_COPY(BookMarksManager)

    BookMarksManagerPrivate *d;
};

} // namespace UkuiFileMetadata

#endif // UKUI_FILE_METADATA_BOOKMARKS_MANAGER_H
