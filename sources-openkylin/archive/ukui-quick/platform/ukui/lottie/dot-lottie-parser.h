/*
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
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

#ifndef UKUI_QUICK_PLATFORM_UKUI_DOT_LOTTIE_PARSER_H
#define UKUI_QUICK_PLATFORM_UKUI_DOT_LOTTIE_PARSER_H

#include "lottie-asset.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

#include <functional>

namespace UkuiQuick {

class UKUI_QUICK_LOTTIE_CORE_PRIVATE_EXPORT DotLottieParser
{
public:
    /// 返回观察协作取消的请求；返回 true 时进行中的工作应尽快中止。
    using CancellationCheck = std::function<bool()>;

    /// 默认使用系统 unzip；内部调用方也可显式提供等价命令路径。
    explicit DotLottieParser(QString unzipExecutable = QString());

    /// v1 统一归档输入预算：压缩输入最多 16 MiB（qrc 与本地来源一致）。
    static constexpr qint64 maxArchiveInputBytes() { return 16LL * 1024LL * 1024LL; }
    /**
     * @brief 普通 Lottie JSON 的固定输入上限（16 MiB），在 QJsonDocument::fromJson 之前强制执行。
     *
     * 理由：图标级动画 JSON 为 KB 量级文本；该上限与归档预算的单条目解压上限
     * 对齐，保证任何来源的 JSON 在进入解析器前被有界化，防止超大或恶意输入
     * 在解析阶段产生无界内存/时间开销。
     */
    static constexpr qint64 maxJsonInputBytes() { return 16LL * 1024LL * 1024LL; }
    /// v1 统一归档输入预算：最多 256 个条目。
    static constexpr int maxArchiveEntries() { return 256; }
    /// v1 统一归档输入预算：单条目解压最多 16 MiB。
    static constexpr qint64 maxExpandedEntryBytes() { return 16LL * 1024LL * 1024LL; }
    /// v1 统一归档输入预算：总解压最多 64 MiB。
    static constexpr qint64 maxTotalExpandedBytes() { return 64LL * 1024LL * 1024LL; }
    /// v1 统一归档输入预算：最大压缩比 100:1。
    static constexpr qint64 maxCompressionRatio() { return 100; }
    /// 单个动画来源允许的主题数量上限；超过时整体失败，不会悄悄截断。
    static constexpr int maxSelectableThemes() { return 256; }
    /// 一个动画最多包含 256 条图片 asset 声明；重复引用同一路径也会计数。
    static constexpr int maxImageResources() { return 256; }

    /// 解析普通 Lottie JSON 文件并构造资产描述。
    LottieAsset createJsonAsset(const QString &sourcePath,
                                const CancellationCheck &isCancelled = {}) const;
    /// 解析已经限额读取的普通 Lottie JSON，并让资产共享这份不可变数据。
    LottieAsset createJsonAsset(const QString &sourcePath,
                                const QByteArray &animationData,
                                const CancellationCheck &isCancelled = {}) const;
    /// 解析已安全解压的 dotLottie 目录。
    LottieAsset parseExtractedDirectory(const QString &sourcePath,
                                        const QString &extractDirPath,
                                        const QString &requestedTheme = QString(),
                                        QString *errorString = nullptr,
                                        const CancellationCheck &isCancelled = {}) const;
    /// 从指定归档路径提取并解析动画，可用于资源快照场景。
    LottieAsset extractArchiveAndParse(const QString &sourcePath,
                                       const QString &archivePath,
                                       const QString &extractDirPath,
                                       const QString &requestedTheme = QString(),
                                       QString *errorString = nullptr,
                                       const CancellationCheck &isCancelled = {}) const;

private:
    /// 根据 manifest 内容选择并构造动画资产。
    LottieAsset parseManifestAsset(const QString &sourcePath,
                                   const QString &canonicalExtractDir,
                                   const QString &manifestPath,
                                   const QStringList &directoryThemes,
                                   const QString &requestedTheme,
                                   QString *errorString,
                                   const CancellationCheck &isCancelled = {}) const;
    /// 根据单条 manifest 动画定义构造动画资产。
    LottieAsset createManifestAnimationAsset(const QString &sourcePath,
                                             const QString &canonicalExtractDir,
                                             const QString &animationDirectory,
                                             const QStringList &themes,
                                             const QString &manifestInitialThemeId,
                                             const QJsonObject &animation,
                                             const QString &requestedTheme,
                                             const CancellationCheck &isCancelled = {}) const;
    /// 在缺少 manifest 时扫描目录并构造第一个可用动画资产。
    LottieAsset findFallbackAnimationAsset(const QString &sourcePath,
                                           const QString &canonicalExtractDir,
                                           const QStringList &directoryThemes,
                                           const QString &requestedTheme,
                                           QString *errorString,
                                           const CancellationCheck &isCancelled = {}) const;
    /// 去除动画或主题标识末尾的 .json 后缀。
    static QString withoutJsonSuffix(QString id);
    /// 从 JSON 数组中提取字符串元素。
    static QStringList stringArray(const QJsonArray &array);
    /// 扫描解压目录中的主题文件并返回主题标识。
    static QStringList themesFromDirectory(const QString &extractDirPath);
    /// 从清单 themes 字段生成主题标识。
    static QStringList themesFromManifest(const QJsonArray &themes);
    /// 将目录中发现、但清单未列出的主题追加到结果中。
    static void appendMissingThemes(QStringList &themes, const QStringList &fallbackThemes);
    /// 将 dotLottie 主题规则转换为 ThorVG 使用的主题替换 JSON。
    static QByteArray themeToSlotJson(const QJsonObject &themeObject, const QString &activeAnimationId);
    /// 将一条主题规则转换为 ThorVG 需要的替换值。
    static QJsonValue themeRuleToSlotValue(const QJsonObject &rule);
    /// 判断主题规则是否适用于当前动画条目。
    static bool shouldProcessThemeRule(const QJsonObject &rule, const QString &activeAnimationId);
    /// 将受限归档安全解压到指定目录，支持协作取消并执行归档预算。
    bool unzipToDirectory(const QString &sourcePath,
                          const QString &extractDirPath,
                          QString *errorString,
                          const CancellationCheck &isCancelled = {}) const;

    /// 读取主题文件，并预先缓存转换后的 ThorVG 主题替换 JSON。
    void cacheThemeSlots(LottieAsset &asset, const CancellationCheck &isCancelled = {}) const;

    QString m_unzipExecutable;
};

} // namespace UkuiQuick

#endif // UKUI_QUICK_PLATFORM_UKUI_DOT_LOTTIE_PARSER_H
