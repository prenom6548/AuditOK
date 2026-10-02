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

#ifndef UKUI_QUICK_PLATFORM_UKUI_LOTTIE_CACHE_MANAGER_H
#define UKUI_QUICK_PLATFORM_UKUI_LOTTIE_CACHE_MANAGER_H

#include "lottie-asset.h"
#include "lottie-source-reference.h"

#include <QFileInfo>
#include <QScopedPointer>
#include <QString>

#include <functional>

namespace UkuiQuick {

class LottieCacheManagerPrivate;

struct LottieAssetRequestResult
{
    LottieAsset asset;
    // 只有候选文件确实不存在时才为 true。文件过大、格式错误、解压失败或
    // 请求取消时都为 false，调用方不能把这些错误误当成静态图标回退。
    bool sourceNotFound = false;
};

class UKUI_QUICK_LOTTIE_CORE_PRIVATE_EXPORT LottieCacheManager
{
public:
    /// 返回观察协作取消的请求；返回 true 时进行中的加载应尽快中止。
    using CancellationCheck = std::function<bool()>;

    static LottieCacheManager *instance();
    ~LottieCacheManager();

    /**
     * @brief 加载、验证并缓存 Lottie 动画。
     *
     * 普通 JSON 只做一次有上限的读取，同时计算 SHA-256，随后由资产自己持有
     * 这份内存数据。本地 dotLottie 由规范 URI 和文件 stat 版本直接定位已解压
     * 目录：命中只检查元数据，未命中才建立一份有上限、自动删除的临时归档
     * 供 unzip 使用。qrc dotLottie 的临时归档和解压目录都只属于当前进程，
     * 不会写入跨进程缓存。
     *
     * JSON 和进程内 qrc 资源可按内容共享解析结果。本地 dotLottie 则按 URI
     * 保持来源隔离。解析后还会从本次来源绑定外部或内嵌图片，并在交付前检查
     * 编码字节、格式、尺寸和总像素。每个逻辑资源按搜索顺序保存有界候选快照，
     * 同一实际路径不会重复保存；声明数量和解码像素按每条图片 asset 声明及候选
     * 计费。渲染线程只尝试这些固定内存快照，不会重新打开来源路径。
     *
     * 所有输入在解析前都有固定大小上限。成功结果的版本号同时包含动画和图片
     * 候选快照的顺序和身份，因此替换外部图片也会让帧缓存失效。
     * 如果调用方请求取消，复制、等待和解压会尽快停止；临时文件会被清理，不会
     * 交付半成品，也不会把取消记成加载失败。
     */
    // 只有文件不存在时，Icon 才能改试同名静态图标；其他失败仍按 Lottie
    // 加载错误处理。主题属于各订阅者，不参与一次共享的 source 加载。
    LottieAssetRequestResult requestAsset(
        const LottieSourceReference &source,
        const CancellationCheck &isCancelled = {});

private:
    LottieCacheManager();

    // 本地 dotLottie 以规范 URI 和 stat 版本寻址。磁盘命中不读取归档；只有
    // 未命中时才建立受限、自动删除的临时副本供外部 unzip 使用。
    LottieAssetRequestResult requestLocalArchive(
        const QString &sourcePath,
        const QString &canonicalPath,
        QFileInfo sourceInfo,
        const CancellationCheck &isCancelled);
    // qrc 没有可跨进程信任的 stat 身份。它在当前进程中受限复制并按内容区分，
    // 临时归档和解压结果都只保存在进程私有目录。
    LottieAssetRequestResult requestResourceArchive(
        const QString &sourcePath,
        QFileInfo sourceInfo,
        const CancellationCheck &isCancelled);

    // 从已经限额读取的不可变内存数据加载普通 JSON 动画。
    LottieAsset loadJsonAsset(const QString &sourcePath,
                              const QByteArray &animationData,
                              const QByteArray &contentDigest,
                              const CancellationCheck &isCancelled = {});
    // 从本次请求的稳定临时副本检查并解压 dotLottie 动画。
    LottieAsset loadDotLottieAsset(const QString &sourcePath,
                                    const QString &archiveInputPath,
                                    const QByteArray &revisionDigest,
                                    const QString &extractDir,
                                    const QString &cacheRoot,
                                    const QSharedPointer<QTemporaryDir> &storageOwner,
                                    const CancellationCheck &isCancelled = {});
    // 让动画资产在使用期间阻止对应磁盘缓存被删除。
    bool attachDiskCacheLease(
        LottieAsset &asset,
        const QSharedPointer<QTemporaryDir> &storageOwner = {});
    // 记录一次加载失败，在短时间内避免对同一坏文件反复重试。
    void recordFailedLoad(const QString &key,
                          const QFileInfo &sourceInfo,
                          bool versionKeyed);
    // 复用解析结果后，重新设置与原来源有关的目录。这样相同动画内容可以共享，
    // 但外部图片仍从本次请求自己的目录查找。
    void rebindAssetToSource(LottieAsset &asset, const QString &sourcePath) const;
    // 复制共享的解析结果，完成本次来源的图片绑定后再交付。所有缓存命中和
    // 首次加载路径都必须经过这里，不能发布尚未绑定资源的资产。
    LottieAssetRequestResult deliverBoundAsset(
        const LottieAsset &parsedAsset,
        const QString &sourcePath,
        const CancellationCheck &isCancelled) const;
    // 读取并检查 JSON 声明的图片，把固定字节绑定到资产，并把资源摘要纳入
    // 最终 revision。返回后 renderer 不再访问原图片路径。
    bool bindImageResources(LottieAsset &asset,
                            const QString &sourcePath,
                            const CancellationCheck &isCancelled) const;
    // 返回经过检查的共享缓存根目录。它遵守 XDG_CACHE_HOME，默认位于
    // ~/.cache/ukui-quick/lottie。
    QString cacheRootDir() const;
    // 根据原来源和版本摘要生成独立解压目录，新旧代际不会互相覆盖。
    QString cacheExtractDirForRevision(const QString &sourcePath, const QString &revisionHex) const;
    // 清理过期临时文件和超出容量限制的磁盘缓存。
    bool maintainDiskCache(const QString &cacheRoot, const QString &protectedPath);
    // 删除一份 dotLottie 的本地解压目录。
    bool removeExtractGeneration(const QString &generationParent, const QString &extractDir);
    // 移除某个目录对应的内存缓存；没有动画仍在使用时，允许删除磁盘数据。
    bool retireCachedAssetsUnderDirectory(const QString &dirPath);
    QScopedPointer<LottieCacheManagerPrivate> d;
};

} // namespace UkuiQuick

#endif // UKUI_QUICK_PLATFORM_UKUI_LOTTIE_CACHE_MANAGER_H
