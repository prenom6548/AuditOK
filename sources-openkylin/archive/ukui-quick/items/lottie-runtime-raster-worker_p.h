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
 */

#ifndef UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_RASTER_WORKER_P_H
#define UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_RASTER_WORKER_P_H

#include <QImage>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QThread>

#include <functional>
#include <memory>

#include "lottie-internal-visibility_p.h"
#include "lottie-runtime-diagnostics_p.h"
#include "ukui/lottie/lottie-asset.h"

namespace UkuiQuick {

namespace LottieRuntimeRasterSessionCache {
constexpr int kDefaultEntries = 16;
constexpr int kMinimumEntries = 1;
constexpr int kMaximumEntries = 16;
} // namespace LottieRuntimeRasterSessionCache

class LottieRuntimeService;
class LottieRuntimeFrameScheduler;

/**
 * @brief 栅格线程生成一帧所需的完整输入。
 *
 * asset 保存 assetRevision 对应的已验证动画内容。请求进入栅格线程前，服务
 * 已经确认主题有效、帧号在范围内，并把尺寸换算成实际像素。
 */
struct UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeFrameRequest
{
    LottieAssetRevision assetRevision;
    LottieAsset asset;
    QString effectiveTheme;
    int frame = 0;
    QSize physicalPixelSize;
};

/**
 * @brief 栅格会话的身份。
 *
 * 会话只绑定不可变的动画内容版本和实际主题。帧号、输出尺寸以及宿主
 * 都是一次请求的属性，不会导致 ThorVG 会话重新 load。
 */
struct UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeSessionKey
{
    LottieAssetRevision assetRevision;
    QString effectiveTheme;

    bool operator==(const LottieRuntimeSessionKey &other) const
    {
        return assetRevision == other.assetRevision &&
            effectiveTheme == other.effectiveTheme;
    }
};

/**
 * @brief 从栅格线程交回应用线程的只读 QImage。
 *
 * QImage 由只读共享指针持有。调用方可以读取像素，但不能修改它；这里也不会
 * 传递 QSGTexture，纹理由每个窗口稍后自行创建。
 */
class UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieImmutableFrame
{
public:
    LottieImmutableFrame() = default;

    bool isValid() const;
    QSize size() const;
    const QImage &image() const;

private:
    friend class LottieRuntimeRasterWorker;
    explicit LottieImmutableFrame(QImage image);

    QSharedPointer<const QImage> m_image;
};

struct UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeFrameResult
{
    enum class Failure {
        None,
        DeterministicTheme,
        TransientTheme,
        Raster
    };

    Failure failure = Failure::Raster;
    LottieAssetRevision assetRevision;
    QString effectiveTheme;
    int frameNumber = 0;
    QSize physicalPixelSize;
    // 测试构建记录栅格耗时；生产构建中的空实现不占用结果空间。
    [[no_unique_address]] LottieRuntimeRasterDiagnostics diagnostics;
    LottieImmutableFrame frame;
};

/**
 * @brief 只在唯一栅格线程中使用的 ThorVG 渲染会话。
 *
 * 会话不会交给组件、其他线程或界面线程。栅格线程按活跃 Host demand 维护
 * 稳定 resident 集合，超出容量的请求使用一次性 transient 会话；任何失败
 * 都只丢弃处理当前请求的那一个会话。
 */
class UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRasterSession
{
public:
    virtual ~LottieRasterSession() = default;

    virtual LottieRuntimeFrameResult::Failure render(
        const LottieRuntimeFrameRequest &request, QImage *image) = 0;
};

class UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeRasterWorker final : public QThread
{
public:
    using Completion = std::function<void(const LottieRuntimeFrameResult &)>;

    ~LottieRuntimeRasterWorker() override;

    bool enqueue(const LottieRuntimeFrameRequest &request, Completion completion);
    bool retainSession(const LottieRuntimeSessionKey &key);
    bool releaseSession(const LottieRuntimeSessionKey &key);
    void stop();
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    // benchmark 使用的内部入口，让下一项任务从全新的会话缓存开始。
    void testResetSessions();
#endif

private:
    friend class LottieRuntimeService;
    friend class LottieRuntimeFrameScheduler;

    explicit LottieRuntimeRasterWorker(int maximumSessions,
                                       QObject *deliveryContext);

    class Private;
    std::unique_ptr<Private> d;

    void run() override;
};

// 测试中临时替换 ThorVG 会话的作用域对象。它必须先于后台服务和帧请求创建，
// 并且只能在 destroyInstance() 已停止所有栅格线程后销毁。栅格线程会复制工厂
// 函数，因此析构时还会确认没有线程继续使用这份副本。该类型只在私有测试头中，
// 不注册到 QML，也不会安装或成为受支持的 C++ 接口。
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
class UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeRasterSessionTestScope
{
public:
    using Factory = std::function<std::unique_ptr<LottieRasterSession>()>;

    explicit LottieRuntimeRasterSessionTestScope(Factory factory);
    ~LottieRuntimeRasterSessionTestScope();

    LottieRuntimeRasterSessionTestScope(const LottieRuntimeRasterSessionTestScope &) = delete;
    LottieRuntimeRasterSessionTestScope &operator=(const LottieRuntimeRasterSessionTestScope &) = delete;
};
#endif

} // namespace UkuiQuick

#endif // UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_RASTER_WORKER_P_H
