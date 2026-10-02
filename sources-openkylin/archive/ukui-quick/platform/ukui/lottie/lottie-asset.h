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

#ifndef UKUI_QUICK_PLATFORM_UKUI_LOTTIE_ASSET_H
#define UKUI_QUICK_PLATFORM_UKUI_LOTTIE_ASSET_H

#include "lottie-core-visibility_p.h"

#include <QByteArray>
#include <QList>
#include <QSharedDataPointer>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QtGlobal>

class QTemporaryDir;

namespace UkuiQuick {

class LottieAssetPrivate;
class LottieCacheManager;
class ParsedAssetCache;

// asset worker 已经读完并检查过的一张图片。渲染线程只使用这里的固定字节，
// 不会再打开原文件或 qrc 路径。
struct UKUI_QUICK_LOTTIE_CORE_PRIVATE_EXPORT LottieBoundImageResource
{
    QByteArray data;
    QByteArray format;
    QSize pixelSize;
    int frameCount = 1;

    bool isValid() const
    {
        return !data.isEmpty() && !format.isEmpty() && pixelSize.isValid() &&
            frameCount > 0;
    }
};

// 只要还有动画资产持有这个对象，对应的磁盘缓存就不能删除。
class UKUI_QUICK_LOTTIE_CORE_PRIVATE_EXPORT LottieAssetCacheLease
{
public:
    explicit LottieAssetCacheLease(
        const QString &extractDir,
        const QSharedPointer<QTemporaryDir> &storageOwner = {});
    ~LottieAssetCacheLease();

    LottieAssetCacheLease(const LottieAssetCacheLease &) = delete;
    LottieAssetCacheLease &operator=(const LottieAssetCacheLease &) = delete;

    bool isValid() const;

private:
    int m_fileDescriptor = -1;
    QSharedPointer<QTemporaryDir> m_storageOwner;
};

/**
 * @brief 一份已验证动画资产的版本号。
 *
 * 动画 JSON 或 dotLottie 先产生 source 版本；如果动画引用图片，asset worker
 * 还会把每张图片的固定字节、格式和像素尺寸加入最终版本。只要动画或图片发生
 * 变化，就不能复用旧帧和旧栅格会话；完整渲染输入相同的资产仍可安全共享。
 */
class UKUI_QUICK_LOTTIE_CORE_PRIVATE_EXPORT LottieAssetRevision
{
public:
    /// 创建一个无效的资产版本。
    LottieAssetRevision() = default;

    /// 返回这个版本是否来自一份成功验证的动画资产。
    bool isValid() const;

    bool operator==(const LottieAssetRevision &other) const;
    bool operator!=(const LottieAssetRevision &other) const;

private:
    friend class LottieAsset;
    friend class LottieCacheManager;

    /// 根据已验证的 source 或内容身份摘要创建版本号。
    static LottieAssetRevision fromIdentityDigest(const QByteArray &identityDigest);

    QByteArray m_identityDigest;
};

/**
 * @brief 保存已解析 Lottie 或 dotLottie 动画的数据。
 *
 * 保存动画文件、资源目录和主题元数据，供渲染器安全加载资源。
 */
class UKUI_QUICK_LOTTIE_CORE_PRIVATE_EXPORT LottieAsset
{
public:
    /// 创建一个无效的空资产。
    LottieAsset();
    /// 复制资产及其共享私有数据。
    LottieAsset(const LottieAsset &other);
    /// 复制赋值资产及其共享私有数据。
    LottieAsset &operator=(const LottieAsset &other);
    /// 销毁资产。
    ~LottieAsset();

    /// 返回动画 JSON 路径或不可变内存数据是否已准备就绪。
    bool isValid() const;

    /// 返回动画资产版本；资产尚未验证成功时返回无效版本。
    LottieAssetRevision revision() const;

    /// 返回动画总帧数（由 op - ip 派生），未解析时为 0。
    int frameCount() const;
    /// 保存动画总帧数。
    void setFrameCount(int frameCount);

    /// 返回动画帧率（fr），未解析时为 0。
    qreal frameRate() const;
    /// 保存动画帧率。
    void setFrameRate(qreal frameRate);

    /// 返回可供渲染器读取的动画 JSON 路径。
    QString animationJsonPath() const;
    /// 保存可供渲染器读取的动画 JSON 路径。
    void setAnimationJsonPath(const QString &animationJsonPath);

    /// 返回由资产直接持有的不可变动画 JSON，包括从 dotLottie 选中的动画数据。
    QByteArray animationJsonData() const;
    /// 保存已验证动画的不可变 JSON 数据。
    void setAnimationJsonData(const QByteArray &animationJsonData);

    /// 返回动画图片资源的主目录。
    QString resourceDir() const;
    /// 设置主资源目录，并加入资源搜索目录。
    void setResourceDir(const QString &resourceDir);

    /// 返回 dotLottie 解压目录。
    QString extractDir() const;
    /// 保存 dotLottie 解压目录。
    void setExtractDir(const QString &extractDir);

    /// 返回最终选中的主题标识。
    QString theme() const;
    /// 保存最终选中的主题标识。
    void setTheme(const QString &theme);

    /// 返回清单声明的默认主题标识。
    QString manifestInitialThemeId() const;
    /// 保存清单声明的默认主题标识。
    void setManifestInitialThemeId(const QString &themeId);

    /// 返回动画条目声明的默认主题标识。
    QString animationInitialThemeId() const;
    /// 保存动画条目声明的默认主题标识。
    void setAnimationInitialThemeId(const QString &themeId);

    /// 返回可用主题标识。
    QStringList availableThemes() const;
    /// 保存可用主题标识。
    void setAvailableThemes(const QStringList &themes);

    /// 返回当前选中的动画条目标识。
    QString activeAnimationId() const;
    /// 保存当前选中的动画条目标识。
    void setActiveAnimationId(const QString &animationId);

    /// 返回当前动画条目允许使用的主题标识。
    QStringList activeAnimationThemes() const;
    /// 保存当前动画条目允许使用的主题标识。
    void setActiveAnimationThemes(const QStringList &themes);

    /// 返回当前动画是否限制主题范围。
    bool activeAnimationRestrictsThemes() const;
    /// 设置当前动画是否限制主题范围。
    void setActiveAnimationRestrictsThemes(bool restricts);

    /// 返回解析器显式发现的唯一图片资源路径。
    QStringList imageResourcePaths() const;
    /// 批量设置解析器发现的图片声明；返回路径仍去重，但会保留每条路径的引用次数。
    void setImageResourcePaths(const QStringList &paths);
    /// 追加一条图片声明。
    void addImageResourcePath(const QString &path);
    /// 返回动画中引用指定逻辑路径的图片声明数量。
    int imageResourceReferenceCount(const QString &path) const;
    /// 返回动画中的图片声明总数，重复引用也会计数。
    int imageResourceDeclarationCount() const;

    /// 返回用于查找外部图片的目录列表。
    QStringList resourceSearchDirs() const;
    /// 批量设置图片资源搜索目录。
    void setResourceSearchDirs(const QStringList &dirs);
    /// 追加一条图片资源搜索目录。
    void addResourceSearchDir(const QString &dir);

    /// 缓存指定主题转换后的 ThorVG 主题替换 JSON。
    void setThemeSlotJson(const QString &themeId, const QByteArray &slotJson);
    /// 获取指定主题的 ThorVG 主题替换 JSON。
    QByteArray themeSlotJson(const QString &themeId) const;

    /// 判断主题是否符合全局和当前动画条目的限制。
    bool canUseTheme(const QString &themeId) const;
    /// 按请求、清单、动画和可用列表顺序选择优先主题。
    QString preferredTheme(const QString &requestedTheme = QString()) const;
    /// 返回位于可信资源目录内的图片候选路径。
    QStringList imagePathCandidates(const QString &sourcePath) const;

    /// 返回 asset worker 为逻辑资源名保存的第一个固定图片候选。
    LottieBoundImageResource boundImageResource(const QString &sourcePath) const;
    /// 按资源搜索顺序返回全部已验证图片快照，供 ThorVG 逐个尝试。
    QList<LottieBoundImageResource> boundImageResourceCandidates(
        const QString &sourcePath) const;
    /// 返回全部图片快照占用的编码字节数；别名不会重复计数。
    qint64 boundImageResourceBytes() const;

    // 记录某个资产版本中已经确认无法应用的主题。这个小型位图由该版本的所有
    // 活跃副本共享，但它只是失败记录，不会改变帧缓存使用的资产版本。
    int selectableThemeOrdinal(const QString &themeId) const;
    bool hasDeterministicThemeFailure(int themeOrdinal) const;
    void rememberDeterministicThemeFailure(int themeOrdinal) const;

    // 返回上述主题失败位图最多能记录多少个主题。
    int themeFailureStateCapacity() const;

private:
    friend class LottieCacheManager;
    friend class ParsedAssetCache;

    void setDiskCacheLease(const QSharedPointer<LottieAssetCacheLease> &lease);
    /// 保存验证成功后生成的资产版本。
    void setRevision(const LottieAssetRevision &revision);
    void clearBoundImageResources();
    void setBoundImageResources(
        const QString &sourcePath,
        const QList<LottieBoundImageResource> &resources);

    /// 保存全部资产数据；复制 LottieAsset 时这些只读数据会共享。
    QSharedDataPointer<LottieAssetPrivate> d;
};

} // namespace UkuiQuick

#endif // UKUI_QUICK_PLATFORM_UKUI_LOTTIE_ASSET_H
