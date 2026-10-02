/*
 * Copyright (C) 2026, Kylin Software Co., Ltd.
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

#ifndef UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_SERVICE_P_H
#define UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_SERVICE_P_H

#include <QCoreApplication>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtGlobal>

#include <functional>

#include "lottie-internal-visibility_p.h"
#include "lottie-runtime-frame-scheduler_p.h"
#include "lottie-runtime-raster-worker_p.h"
#include "ukui/lottie/lottie-asset.h"
#include "ukui/lottie/lottie-source-reference.h"

namespace UkuiQuick {

// 在应用退出时关闭动画后台线程。该函数只供插件内部使用，不是公开接口。
void UKUI_QUICK_LOTTIE_INTERNAL_EXPORT installLottieRuntimeApplicationShutdownHook();

class LottieRuntimeService;

/// 后台加载只需要来源信息。主题等每个 Icon/LottieItem 各自选择的状态，会在加载完成后
/// 分别处理，不会让同一份文件被重复读取和解析。
struct UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeSourceRequest
{
    LottieSourceReference source;
};

struct UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeSourceResult
{
    bool loaded = false;
    bool sourceNotFound = false;
    LottieAsset asset;
};

/**
 * @brief Icon 或 LottieItem 用来接收异步结果的订阅句柄。
 *
 * 组件只持有这个句柄，不直接拥有后台线程或队列。相同来源的请求仍在排队时
 * 可以共用一次加载；加载一旦开始，后来的请求会重新读取文件。这样即使同一路径
 * 的文件刚被替换，新请求也能得到新内容。组件换来源或销毁时，只取消自己的
 * 订阅；最后一个订阅者离开后，公共加载任务才会被移除或通知取消。已经过期的
 * 结果不会交给组件，取消也不会显示为加载错误。
 */
class UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeSubscription
{
public:
    LottieRuntimeSubscription() = default;
    LottieRuntimeSubscription(LottieRuntimeSubscription &&other) noexcept;
    LottieRuntimeSubscription &operator=(LottieRuntimeSubscription &&other) noexcept;
    ~LottieRuntimeSubscription();

    LottieRuntimeSubscription(const LottieRuntimeSubscription &) = delete;
    LottieRuntimeSubscription &operator=(const LottieRuntimeSubscription &) = delete;

    bool isValid() const;

private:
    friend class LottieRuntimeService;
    LottieRuntimeSubscription(LottieRuntimeService *service, quint64 id);

    QPointer<LottieRuntimeService> m_service;
    quint64 m_id = 0;
};

class LottieRuntimeServicePrivate;

/**
 * @brief 整个应用共用的 Lottie 后台服务。
 *
 * 每个应用最多创建一个实例，并随 QCoreApplication 一起销毁。GUI 线程只提交
 * 来源描述，不读取文件。唯一的后台加载线程串行完成路径解析、复制、解压和
 * 解析，并为本次读取生成固定不变的内容副本。仍在排队的相同来源可以合并；
 * 已开始加载的任务不再接收后来请求，以免同一路径的新旧内容混在一起。
 *
 * 加载完成后，结果回到应用线程，只交给仍在等待它的订阅者。主题等组件自己的
 * 选择在这一步分别处理。某次加载失败只影响该请求，不会关闭整个服务。应用退出
 * 时，服务先拒绝新请求、清空排队任务，再通知运行中的任务尽快取消并等待线程退出。
 *
 * 该类是内部实现，不注册 QML 类型，不安装头文件，不构成公开 C++ ABI。
 */
class UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeService : public QObject
{
    Q_OBJECT
public:
    /// 观察当前资产工作是否被要求协作取消；返回 true 时执行器应尽快中止。
    using SourceCancellation = std::function<bool()>;
    using SourceExecutor = std::function<LottieRuntimeSourceResult(const LottieRuntimeSourceRequest &,
                                                                   const SourceCancellation &)>;
    using SourceCompletion = std::function<void(const LottieRuntimeSourceResult &)>;
    using FrameCompletion = LottieRuntimeRasterWorker::Completion;

    /// 返回应用共用的后台服务；首次调用时创建。没有 QCoreApplication 时返回 nullptr。
    static LottieRuntimeService *instance();
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    /// 返回当前应用是否已经创建后台服务。
    static bool hasInstance();
    /// 停止并销毁现有实例，供测试恢复到初始状态。
    static void destroyInstance();
#endif

    ~LottieRuntimeService() override;

    /// 创建一个订阅；服务正在停止时返回无效订阅。
    LottieRuntimeSubscription subscribe();

    /**
     * @brief 为一个订阅异步加载动画来源。
     *
     * 返回 false 表示服务正在停止、订阅无效或来源为空，此时不会调用完成回调。
     * 接受请求后，订阅会先离开旧任务；如果它是最后一个等待者，旧任务会被移除
     * 或通知取消。新请求只能加入尚未开始的同来源任务；任务已经开始时会另建一次
     * 加载，确保读到请求当时的文件内容。正常完成时，回调在应用线程调用一次。
     * 如果订阅已销毁、换了来源、任务被取消或服务已停止，则不再调用回调。
     */
    bool requestSource(const LottieRuntimeSubscription &subscription,
                       const LottieSourceReference &source, const QString &theme,
                       SourceCompletion completion);

    /**
     * @brief 请求唯一的栅格线程生成一帧 QImage。
     *
     * 请求必须同时携带有效的内容版本和对应动画资产。服务会把帧号限制在有效
     * 范围内，并检查主题和实际像素尺寸，然后才加入队列。完成回调在应用线程
     * 执行，只返回不可修改的 QImage，不返回场景图纹理。
     */
    bool requestFrame(const LottieRuntimeSubscription &subscription,
                      const LottieRuntimeFrameRequest &request,
                      FrameCompletion completion);

    /**
     * @brief 暂停一个 Host 的当前帧兴趣和 session demand。
     *
     * source subscription、已发布 metadata 和 facade 状态继续保留；Host
     * 恢复可渲染后由 requestFrame() 惰性重新建立需求。该入口只供内部
     * Host adapter 使用，不注册 QML，也不构成安装后的公共 ABI。
     */
    void suspendFrame(const LottieRuntimeSubscription &subscription);

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    // 以下入口只给内部测试使用，不会注册到 QML，也不会安装。
    bool requestSource(const LottieRuntimeSubscription &subscription,
                       const QString &source, const QString &theme,
                       SourceCompletion completion);
    bool isStopping() const;
    LottieRuntimeFrameSchedulerStats testFrameSchedulerStats() const;
    Q_INVOKABLE LottieRuntimeFrameSchedulerStats frameSchedulerDiagnostics() const;
    /// 只统计文件加载线程的耗时，不包括帧渲染和界面提交。
    Q_INVOKABLE qint64 sourceLoadingP95Nanoseconds() const;
    Q_INVOKABLE void resetFrameSchedulerDiagnostics();
    // benchmark 使用的内部入口；仍有帧在处理或等待交付时拒绝清空缓存。
    Q_INVOKABLE bool testResetFrameSchedulerCache();
    Q_INVOKABLE int testAssetWorkerCount() const;
    Q_INVOKABLE int testSourceCompletionCount() const;
    Q_INVOKABLE int testSubscriptionCount() const;
    Q_INVOKABLE int testSourceRequestCount() const;
    Q_INVOKABLE int testFrameRequestCount() const;
    Q_INVOKABLE void testSetStoppingTransitionHook(quintptr hook);
    void testHoldFrameSchedulerGuiBatch(bool hold);
    void testFlushFrameSchedulerGuiBatch();
#endif

    /// 停止服务：拒绝新请求、丢弃排队任务、通知运行中的任务取消，并等待线程退出。
    /// 线程完全退出之前，服务使用的状态和回调接收对象都会继续存在。
    /// 正常情况下由 QCoreApplication::aboutToQuit 触发，测试也会直接调用。
    void stopForShutdown();

private:
    friend class LottieRuntimeSubscription;
    explicit LottieRuntimeService(const SourceExecutor &executor, QObject *parent);

    void unsubscribe(quint64 subscriptionId);
    void deliverJobResult(quint64 jobId, const LottieRuntimeSourceResult &result);

    LottieRuntimeServicePrivate *d;
};

/**
 * @brief 测试中临时替换文件加载过程的作用域对象。
 *
 * 创建时先销毁现有服务，让下一次创建的服务使用测试提供的加载函数；析构时
 * 再销毁测试服务并恢复默认加载函数。必须在创建 Icon/LottieItem 或订阅之前使用。
 * 该类型不注册到 QML，也不会安装。
 */
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
class UKUI_QUICK_LOTTIE_INTERNAL_EXPORT LottieRuntimeServiceTestScope
{
public:
    using SourceExecutor = LottieRuntimeService::SourceExecutor;

    explicit LottieRuntimeServiceTestScope(SourceExecutor executor = {});
    ~LottieRuntimeServiceTestScope();

    LottieRuntimeServiceTestScope(const LottieRuntimeServiceTestScope &) = delete;
    LottieRuntimeServiceTestScope &operator=(const LottieRuntimeServiceTestScope &) = delete;
};
#endif

} // namespace UkuiQuick

#endif // UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_SERVICE_P_H
