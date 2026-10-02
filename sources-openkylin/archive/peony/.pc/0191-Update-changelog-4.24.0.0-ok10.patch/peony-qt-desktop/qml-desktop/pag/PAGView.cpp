/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making libpag available.
//
//  Copyright (C) 2025 Tencent. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "PAGView.h"

#include <QDebug>
#include <QFileInfo>
#include <QGuiApplication>
#include <QEvent>
#include <QMutexLocker>
#include <QQuickWindow>
#include <QOpenGLContext>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <QSGImageNode>
#include <QScreen>
#include <QUrl>
#include <ffavc/ffavc.h>

namespace pag {

PAGView::PAGView(QQuickItem* parent) : QQuickItem(parent) {
  static bool decoderRegistered = false;
  if (!decoderRegistered) {
    decoderRegistered = true;
    PAGVideoDecoder::RegisterSoftwareDecoderFactory(
        static_cast<SoftwareDecoderFactory*>(ffavc::DecoderFactory::GetHandle()));
  }

  qRegisterMetaType<PAGFrameTicket>("pag::PAGFrameTicket");
  setFlag(ItemHasContents, true);
  drawable_ = GPUDrawable::MakeFrom(this);
  renderThread_.reset(new PAGRenderThread(drawable_));
  renderThread_->moveToThread(renderThread_.get());
  drawable_->moveToThread(renderThread_.get());

  connect(renderThread_.get(), &PAGRenderThread::frameFlushed,
          this, &PAGView::onFrameFlushed, Qt::QueuedConnection);
  connect(renderThread_.get(), &PAGRenderThread::hiddenFramePresented,
          this, &PAGView::onHiddenFramePresented, Qt::QueuedConnection);
  connect(renderThread_.get(), &PAGRenderThread::playbackFinished,
          this, &PAGView::onPlaybackFinished, Qt::QueuedConnection);
  connect(renderThread_.get(), &PAGRenderThread::loadFailed,
          this, &PAGView::onLoadFailed, Qt::QueuedConnection);
  connect(renderThread_.get(), &PAGRenderThread::safeToUnloadWorker,
          this, &PAGView::onWorkerSafeToUnload, Qt::QueuedConnection);
  connect(renderThread_.get(), &PAGRenderThread::cancelAcknowledged,
          this, &PAGView::onCancelAcknowledged, Qt::QueuedConnection);

  renderThread_->start();
  renderThread_->setPriority(QThread::NormalPriority);
  stabilityTimer_.setSingleShot(true);
  stabilityTimer_.setInterval(200);
  connect(&stabilityTimer_, &QTimer::timeout, this, &PAGView::prepareAfterStableWindow);
  connect(this, &QQuickItem::windowChanged, this, &PAGView::onWindowChanged);
}

PAGView::~PAGView() {
#if 0  // 诊断日志：析构线程与节点/票据挂接状态（排查生命周期问题时启用）
  qDebug().noquote().nospace()
      << "[PAG_GL] event=pagview-destroy-enter this=" << this
      << " drawable=" << drawable_.get()
      << " renderThread=" << renderThread_.get()
      << " thread=" << QThread::currentThread()
      << " nodeAttached=" << nodeAttached_
      << " attachedTicketValid=" << attachedTicketValid_;
#endif
  active_ = false;
  pendingTicketValid_ = false;
  attachedTicketValid_ = false;
  QMetaObject::invokeMethod(renderThread_.get(), "shutDown", Qt::QueuedConnection);
  renderThread_->wait();
#if 0  // 诊断日志：等待渲染线程退出后确认（排查生命周期问题时启用）
  qDebug().noquote().nospace()
      << "[PAG_GL] event=pagview-destroy-after-wait this=" << this
      << " drawable=" << drawable_.get()
      << " thread=" << QThread::currentThread();
#endif
}

int PAGView::scaleModeForOption(const QString& option) const {
  const auto value = option.toLower();
  if (value == QLatin1String("scaled")) {
    return static_cast<int>(PAGScaleMode::Zoom);
  }
  if (value == QLatin1String("zoom")) {
    return static_cast<int>(PAGScaleMode::LetterBox);
  }
  return static_cast<int>(PAGScaleMode::Stretch);
}

// 解锁请求只建立本次会话，首帧准备和可见播放分别由后续阶段驱动
/*
 * 一次解锁会话先准备隐藏首帧，再启动可见播放：
 * 1. prepareUnlock 校验壁纸、主屏和窗口条件，并递增 visibilityEpoch
 * 2. 稳定窗口后由工作线程解码并提交 PrepareHidden 票据
 * 3. 首帧完成场景图交换后，beginVisiblePlayback 才开始提交可见帧
 * 这样登录阶段的窗口尺寸变化不会把首帧直接吞掉
 */
bool PAGView::prepareUnlock(const QString& filePath, const QString& option,
                            qulonglong generation, bool staticReady, bool primary,
                            bool skipStabilityDelay) {
  QMutexLocker locker(&stateMutex_);
  // 节点保留方案下 detach 在 cancelPlayback 内同步完成，此处无需再计算
  // mustDetachOldNode / waitingForDetach_。
  cancelPlayback();
  const auto localPath = filePath.startsWith(QLatin1String("file://"))
                             ? QUrl(filePath).toLocalFile()
                             : filePath;
  const auto normalizedOption = option.toLower();
  const bool supportedOption = normalizedOption == QLatin1String("stretched") ||
                               normalizedOption == QLatin1String("scaled") ||
                               normalizedOption == QLatin1String("zoom");
  if (localPath.isEmpty() || !QFileInfo::exists(localPath) || !staticReady || !primary ||
      !supportedOption) {
    generation_ = generation;
    ++visibilityEpoch_;
    active_ = false;
    cancelling_ = true;
    loadingStarted_ = false;
    // 节点保留（见 updatePaintNode/cancelPlayback），detach 已在 cancelPlayback 同步完成，
    // 无需等待 Qt 销毁节点，waitingForDetach_ 保持 false。
    waitingForDetach_ = false;
    if (!nodeAttached_) {
      QMetaObject::invokeMethod(renderThread_.get(), "cancelEmptySession", Qt::QueuedConnection,
                                Q_ARG(qulonglong, generation_), Q_ARG(qulonglong, windowEpoch_),
                                Q_ARG(qulonglong, visibilityEpoch_));
    }
    return false;
  }

  filePath_ = localPath;
  option_ = normalizedOption;
  generation_ = generation;
  staticReady_ = staticReady;
  primary_ = primary;
  active_ = true;
  cancelling_ = false;
  loadingStarted_ = false;
  skipStabilityDelay_ = skipStabilityDelay;
  visibleStartRetryCount_ = 0;
  visibleFrameRepaintRequested_ = false;
  // 节点保留：detach 已在 cancelPlayback 同步完成，直接进入正常播放流程。
  waitingForDetach_ = false;
  ++visibilityEpoch_;
#if 0  // 会话级日志（默认关闭；需要跟踪解锁会话请求时可切换 #if 1）
  qDebug().noquote().nospace()
      << "[PAG_SESSION] event=unlock-request gen=" << generation_
      << " windowEpoch=" << windowEpoch_
      << " visibilityEpoch=" << visibilityEpoch_
      << " skipStabilityDelay=" << skipStabilityDelay_;
#endif
  Q_EMIT filePathChanged(filePath_);
  Q_EMIT activeChanged(true);
  if (!waitingForDetach_) {
    restartStabilityTimer();
  } else {
    update();
  }
  return true;
}

void PAGView::beginVisiblePlayback(qulonglong generation) {
  // 窗口和场景图就绪后才能启动可见播放，避免首帧在登录阶段被吞掉
  QMutexLocker locker(&stateMutex_);
  if (!active_ || cancelling_ || generation != generation_) {
    return;
  }
  if (!windowIsReady()) {
    if (visibleStartRetryCount_ < 3) {
      ++visibleStartRetryCount_;
      QTimer::singleShot(16, this, [this, generation]() {
        beginVisiblePlayback(generation);
      });
    } else {
      const auto failedGeneration = generation_;
      cancelPlayback();
      Q_EMIT failed(failedGeneration, QStringLiteral("visible window is not ready"));
    }
    return;
  }
  if (!visibleFrameRepaintRequested_) {
    visibleFrameRepaintRequested_ = true;
    update();
    if (window()) {
      window()->update();
    }
  }
  QMetaObject::invokeMethod(renderThread_.get(), "startVisiblePlayback", Qt::QueuedConnection,
                            Q_ARG(qulonglong, generation_), Q_ARG(qulonglong, windowEpoch_),
                            Q_ARG(qulonglong, visibilityEpoch_));
}

void PAGView::requestPendingFrameUpdate() {
  QMutexLocker locker(&stateMutex_);
  if (!active_ || cancelling_) {
    return;
  }
  // 锁屏遮挡期间的更新可能未被场景图消费，解锁时重新请求一次即可
#if 0  // 会话级日志（默认关闭；需要跟踪解锁后补帧请求时可切换 #if 1）
  qDebug().noquote().nospace()
      << "[PAG_SESSION] event=resume-pending-frame gen=" << generation_
      << " pending=" << pendingTicketValid_
      << " attached=" << attachedTicketValid_;
#endif
  update();
}

// mp4 壁纸常驻会话：与 unlock 会话隔离，触发源为 gsettings picture-filename=.mp4
/*
 * locked=true（冷启动已锁定/锁屏期间路径变化/锁屏期间窗口重建）：
 *   首帧 PrepareHidden 完成后不启动可见播放（prepared-paused），解锁由
 *   resumeWallpaperPlayback() 从帧 0 起播——复用 pag 锁屏预加载语义
 * locked=false：首帧就绪后自动进入可见播放（onHiddenFramePresented 内检查 paused_）
 */
bool PAGView::beginWallpaperPlayback(const QString& filePath, bool locked, const QString& option) {
  QMutexLocker locker(&stateMutex_);
  cancelPlayback();
  const auto localPath = filePath.startsWith(QLatin1String("file://"))
                             ? QUrl(filePath).toLocalFile()
                             : filePath;
  if (localPath.isEmpty() || !QFileInfo::exists(localPath)) {
    wallpaperActive_ = false;
    paused_ = false;
    return false;
  }
  wallpaperRestartScheduled_ = false;
  pendingResumeAfterCancel_ = false;  // 新的显式会话意图取代挂起的解锁恢复
  filePath_ = localPath;
  // mp4 壁纸显示方式跟随 picture-options（scaleModeForOption 映射：
  // scaled→Zoom、zoom→LetterBox、其余→Stretch）；空值兜底 scaled 保持原行为
  option_ = option.isEmpty() ? QStringLiteral("scaled") : option;
  ++generation_;
  staticReady_ = true;
  primary_ = true;
  active_ = true;
  cancelling_ = false;
  loadingStarted_ = false;
  wallpaperActive_ = true;
  paused_ = locked;
  // 锁定时窗口已稳定 → 0ms 直载并停于 prepared-paused（等价 pag 锁屏预加载）
  skipStabilityDelay_ = locked;
  visibleStartRetryCount_ = 0;
  visibleFrameRepaintRequested_ = false;
  waitingForDetach_ = false;
  ++visibilityEpoch_;
#if 0  // 会话级日志（默认关闭；需要跟踪会话开始/预加载可切换 #if 1）
  qDebug().noquote().nospace()
      << "[PAG_SESSION] event=wallpaper-request gen=" << generation_
      << " windowEpoch=" << windowEpoch_
      << " visibilityEpoch=" << visibilityEpoch_
      << " locked=" << locked
      << " skipStabilityDelay=" << skipStabilityDelay_;
  qDebug().noquote().nospace()
      << "[PAG_DEBUG] event=begin-wallpaper this=" << this
      << " itemSize=" << width() << "x" << height()
      << " dpr=" << (observedWindow_ ? observedWindow_->devicePixelRatio() : -1.0)
      << " winSize=" << (observedWindow_ ? observedWindow_->width() : 0) << "x"
      << (observedWindow_ ? observedWindow_->height() : 0)
      << " option=" << option_;
#endif
  Q_EMIT filePathChanged(filePath_);
  Q_EMIT activeChanged(true);
  restartStabilityTimer();
  return true;
}

// 停止壁纸会话：清除 wallpaperActive_（自动重启唯一正确的 recheck 依据），走通用取消
void PAGView::stopWallpaperPlayback() {
  QMutexLocker locker(&stateMutex_);
  wallpaperActive_ = false;
  paused_ = false;
  pendingResumeAfterCancel_ = false;  // 停止意图取代挂起的解锁恢复
  cancelPlayback();
}

void PAGView::pauseWallpaperPlayback() {
  QMutexLocker locker(&stateMutex_);
  if (!wallpaperActive_ || cancelling_ || paused_) {
    return;
  }
  paused_ = true;
  // 清空待确认票据：锁屏期间旧票据的 swapped 可能永远不来（屏幕熄灭/合成冻结），
  // 挂着的 attached/pending 会让解锁后的新帧被 onFrameFlushed 丢弃（pending/attached 占用）。
  // 节点上的纹理继续显示锁屏前最后一帧，画面冻结语义不变。
  pendingTicketValid_ = false;
  attachedTicketValid_ = false;
  // 会话已被窗口变化取消（active_=false）时仍置 paused_：挂起中的自动重启必须落在
  // prepared-paused；工作线程无会话则 pause no-op
  if (active_) {
    QMetaObject::invokeMethod(renderThread_.get(), "pause", Qt::QueuedConnection);
  }
}

void PAGView::resumeWallpaperPlayback() {
  QMutexLocker locker(&stateMutex_);
  if (!wallpaperActive_) {
    return;
  }
  // 解锁是既成事实：无论会话是否处于取消中都必须先解除锁定态，否则
  // onCancelAcknowledged 触发的延迟重启会被 maybeRestart 的 paused_ 复查拦下（黑屏根因）
  paused_ = false;
  if (cancelling_) {
    // cancel 的 ack 未回：挂起恢复意图，由 onCancelAcknowledged 确认取消后转为自动重启
    pendingResumeAfterCancel_ = true;
    return;
  }
  if (!active_) {
    // 会话已被窗口/屏幕变化取消：立即排定重启（重启后从 prepared-paused 续为 playing）
    maybeRestartWallpaperAfterCancel();
    return;
  }
  QMetaObject::invokeMethod(renderThread_.get(), "resume", Qt::QueuedConnection);
  if (!visibleFrameRepaintRequested_) {
    visibleFrameRepaintRequested_ = true;
    update();
    if (window()) {
      window()->update();
    }
  }
}

// 窗口/屏幕变化（热插拔、镜像、DPI、可见性、几何）取消壁纸会话后，延迟自动重启
// 重启前复查 wallpaperActive_/paused_（timer 触发时复查，锁屏 200ms 内到达不丢失），
// 幂等合并连续 cancel→restart，防止锁屏期间 DPMS/遮挡反复切换的乒乓
void PAGView::maybeRestartWallpaperAfterCancel() {
  QMutexLocker locker(&stateMutex_);
  if (!wallpaperActive_ || wallpaperRestartScheduled_) {
    return;
  }
  wallpaperRestartScheduled_ = true;
  const auto delay = 200;  // 与 stabilityTimer 同量级
  QTimer::singleShot(delay, this,
                     [this]() {
    QMutexLocker locker(&stateMutex_);
    wallpaperRestartScheduled_ = false;
    if (!wallpaperActive_ || active_ || paused_) {
      return;  // 已停/已重启/锁屏（解锁后 resumeWallpaperPlayback 再排）
    }
    if (!windowIsReady()) {
      maybeRestartWallpaperAfterCancel();  // 窗口暂未就绪，稍后重试
      return;
    }
    beginWallpaperPlayback(filePath_, paused_, option_);
  });
}

// cancelPlayback 的确认回调：清 cancelling_ 并兑现挂起的解锁恢复。
// 此前未连接本信号，cancelling_ 只能等下次 begin/prepareUnlock 复位，
// 锁屏期间会话被窗口事件取消后，解锁 resume 即被永久挡死（黑屏根因）。
// 守卫只比对 generation：窗口事件在 cancel 后才递增 windowEpoch_，连发取消
// 在渲染线程 matches 不过、不会产生第二笔 ack，严格比对会把唯一 ack 拒掉。
void PAGView::onCancelAcknowledged(qulonglong generation, qulonglong windowEpoch,
                                   qulonglong visibilityEpoch) {
  Q_UNUSED(windowEpoch)
  Q_UNUSED(visibilityEpoch)
  QMutexLocker locker(&stateMutex_);
  if (!cancelling_ || generation != cancelGeneration_) {
    return;  // 已被新会话/新取消取代，或属于更早世代的迟到确认
  }
  cancelling_ = false;
  const bool resumeRequested = pendingResumeAfterCancel_;
  pendingResumeAfterCancel_ = false;
  if (!wallpaperActive_) {
    return;  // pag 解锁会话等非壁纸场景：仅复位 cancelling_，不触发自动重启
  }
  // resumeRequested：锁屏期间被取消、解锁在取消确认前到达（本 bug 主路径）
  // !active_ && !paused_：兜底——任何未排定重启的非锁屏取消都在此自愈
  if (resumeRequested || (!active_ && !paused_)) {
    maybeRestartWallpaperAfterCancel();
  }
}


// 取消会话时先清空界面票据，再通知渲染线程停止，防止旧帧回写到新会话
void PAGView::cancelPlayback() {
  QMutexLocker locker(&stateMutex_);
  stabilityTimer_.stop();
  if (!active_ && !nodeAttached_ && !pendingTicketValid_) {
    return;
  }
  const auto oldGeneration = generation_;
  const auto oldWindowEpoch = windowEpoch_;
  const auto oldVisibilityEpoch = visibilityEpoch_;
  const bool workerLoadStarted = loadingStarted_;
  cancelGeneration_ = oldGeneration;
  cancelWindowEpoch_ = oldWindowEpoch;
  cancelVisibilityEpoch_ = oldVisibilityEpoch;
  active_ = false;
  cancelling_ = true;
  loadingStarted_ = false;
  pendingTicketValid_ = false;
  attachedTicketValid_ = false;
  // QSGImageNode 在锁屏/解锁期间保留（见 updatePaintNode），不再通过返回 nullptr
  // 让 Qt 销毁节点。因此 detach 协议在取消时同步完成：清除等待标志并立即通知
  // 工作线程释放 PAG 会话。节点持有的最后一个纹理由 pag 侧 ring 延迟回收，
  // 工作线程 unload 只释放 player/file（CPU 对象），不触碰 Surface/GL 纹理。
  detachPending_ = false;
  waitingForDetach_ = false;
  Q_EMIT activeChanged(false);
  update();
  QMetaObject::invokeMethod(renderThread_.get(), "cancel", Qt::QueuedConnection,
                            Q_ARG(qulonglong, oldGeneration), Q_ARG(qulonglong, oldWindowEpoch),
                            Q_ARG(qulonglong, oldVisibilityEpoch));
  // 通知工作线程 nodeDetached（epoch 守卫 + safeUnloadEmitted_ 保证幂等），
  // 使 unloadIfDetached 能执行资源释放；节点保留与否均需要该通知。
  notifyNodeDetached();
  // 会话未到达工作线程（加载未派发）时，cancel() 在渲染线程可能因三元组不匹配而
  // 静默（render 端状态仍属于上一个会话），必须靠 cancelEmptySession 采纳三元组并
  // 回 cancelAcknowledged；节点保留（nodeAttached_）不构成障碍——cancelEmptySession
  // 内部先查 player/file/ticketInFlight，worker 无资源时只是采纳+确认，无副作用。
  // 缺了它，锁定期间窗口抖动抢占加载派发会把 cancelling_ 永久卡死（与主 bug 同症状）。
  if (!workerLoadStarted && !waitingForDetach_) {
    QMetaObject::invokeMethod(renderThread_.get(), "cancelEmptySession", Qt::QueuedConnection,
                              Q_ARG(qulonglong, oldGeneration),
                              Q_ARG(qulonglong, oldWindowEpoch),
                              Q_ARG(qulonglong, oldVisibilityEpoch));
  }
}

// 窗口尺寸变化会使旧渲染结果失效，递增 windowEpoch 隔离异步回调
void PAGView::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry) {
  QMutexLocker locker(&stateMutex_);
  qDebug().noquote().nospace()
      << "[PAG_DEBUG] event=geometry-changed this=" << this
      << " active=" << active_ << " loading=" << loadingStarted_
      << " new=" << newGeometry.width() << "x" << newGeometry.height()
      << "@" << newGeometry.x() << "," << newGeometry.y()
      << " old=" << oldGeometry.width() << "x" << oldGeometry.height();
  QQuickItem::geometryChanged(newGeometry, oldGeometry);
  if (newGeometry == oldGeometry) {
    return;
  }
  if (active_) {
    if (loadingStarted_) {
      cancelPlayback();
      {
        QMutexLocker locker(&stateMutex_);
        ++windowEpoch_;
      }
      maybeRestartWallpaperAfterCancel();
    } else {
      skipStabilityDelay_ = false;
      ++windowEpoch_;
      restartStabilityTimer();
    }
  }
}


// ---------------------------------------------------------------------------
// 零拷贝就绪探测：QSG 渲染线程检查 EGL dma-buf import 能力，可用则通过
// pag_zc_ready()（libffavc 导出符号）通知 ffavc，并写 PAG_VIDEO_ZERO_COPY_EGL
// 供 tgfx 选库。三态：未设置=自动探测；"0"=强制关闭；"1"=强制启用（跳过探测）。
// ---------------------------------------------------------------------------
static void probeZeroCopyReadyAtRender() {
  static bool probed = false;
  const char* env = std::getenv("PAG_VIDEO_ZERO_COPY");
  if (env != nullptr && std::strcmp(env, "0") == 0) {
    return;  // 逃生舱强制关：不探测，行为与旧版一致
  }
  if (env != nullptr && std::strcmp(env, "1") == 0) {
    return;  // 强制开：跳过能力探测，导入失败由 ffavc 侧 latch 回退兜底
  }
  if (probed) {
    return;
  }
  if (QOpenGLContext::currentContext() == nullptr) {
    // 尚未有 GL current（罕见时序）：不 latch，等下一次 beforeRendering 重试
    return;
  }
  // 进入正式探测即 latch（平台能力不会在进程内变化）；无 GL current 例外已上移
  probed = true;
  // Kylin-wlcom 的 QSG 渲染上下文可能由私有 EGL 直接创建（绕过 glvnd loader，
  // 此时 glvnd 的 eglGetCurrentDisplay 查不到）。依次尝试 glvnd 与已知私有 EGL，
  // 找到"当前线程有 display 且支持 dma-buf import"的实现，并把库名写入
  // PAG_VIDEO_ZERO_COPY_EGL 供 tgfx 消费侧使用同名库做导入。
  const char* eglLibs[] = {
      "libEGL.so.1",       // glvnd loader
      "libEGL_leopard.so.0", "libEGL_fh2m.so.0", "libEGL_fh2m_l.so.0",
      "libEGL_arise.so.0", "libEGL_mesa.so.0", "libEGL_ljm.so.0", "libEGL_mtgpu.so.0",
      nullptr};
  for (int li = 0; eglLibs[li] != nullptr; li++) {
    void* egl = dlopen(eglLibs[li], RTLD_NOW | RTLD_NOLOAD);
    if (egl == nullptr) {
      continue;
    }
    auto eglGetCurrentDisplay = reinterpret_cast<void* (*)()>(dlsym(egl, "eglGetCurrentDisplay"));
    auto eglQueryString = reinterpret_cast<const char* (*)(void*, int)>(dlsym(egl, "eglQueryString"));
    auto eglGetProcAddress = reinterpret_cast<void* (*)(const char*)>(dlsym(egl, "eglGetProcAddress"));
    if (eglGetCurrentDisplay == nullptr || eglQueryString == nullptr || eglGetProcAddress == nullptr ||
        eglGetProcAddress("eglCreateImageKHR") == nullptr) {
      continue;
    }
    void* dpy = eglGetCurrentDisplay();
    if (dpy == nullptr) {
      continue;
    }
    const char* exts = eglQueryString(dpy, 0x3055);  // EGL_EXTENSIONS
    if (exts == nullptr || std::strstr(exts, "EGL_EXT_image_dma_buf_import") == nullptr ||
        std::strstr(exts, "EGL_KHR_image_base") == nullptr) {
      continue;
    }
    // 通知 ffavc 解码线程（新版 libffavc 读 atomic 通道，无 setenv/getenv 竞态；
    // 兼容旧版：同时保留 READY env）。
    setenv("PAG_VIDEO_ZERO_COPY_READY", "1", 1);
    setenv("PAG_VIDEO_ZERO_COPY_EGL", eglLibs[li], 1);
    static void (*zcReadyFn)(int) = []() {
      void* h = dlopen("libffavc.so", RTLD_NOW | RTLD_NOLOAD);
      if (h == nullptr) {
        return static_cast<void (*)(int)>(nullptr);
      }
      return reinterpret_cast<void (*)(int)>(dlsym(h, "pag_zc_ready"));
    }();
    if (zcReadyFn != nullptr) {
      zcReadyFn(1);
    }
    qInfo() << "[PAG_GL] zero-copy READY: EGL dma-buf import available via" << eglLibs[li];
    return;
  }
  qWarning() << "[PAG_GL] zero-copy probe: no EGL implementation with current display + dma-buf import";
}

void PAGView::onWindowChanged(QQuickWindow* window) {
  QMutexLocker locker(&stateMutex_);
  if (active_) {
    cancelPlayback();
    maybeRestartWallpaperAfterCancel();
  }
  if (observedWindow_) {
    observedWindow_->removeEventFilter(this);
    disconnect(observedWindow_, nullptr, this, nullptr);
  }
  observeScreen(nullptr);
  observedWindow_ = window;
  ++windowEpoch_;
  qDebug().noquote().nospace()
      << "[PAG_DEBUG] event=window-changed this=" << this
      << " win=" << (window ? window->width() : 0) << "x" << (window ? window->height() : 0)
      << " dpr=" << (window ? window->devicePixelRatio() : -1.0)
      << " active=" << active_;
  if (!window) {
    cancelPlayback();
    return;
  }
  window->installEventFilter(this);
  // 此处有意使用 DirectConnection：交换操作只能消费紧邻的上一次 updatePaintNode
  // 所挂接的帧凭据，不能消费 GUI 线程队列中无关交换对应的凭据
  connect(window, &QQuickWindow::frameSwapped, this, &PAGView::onFrameSwapped,
          Qt::DirectConnection);
  // QSG 渲染线程、GL current：零拷贝能力探测现场（进程级一次）。
  connect(window, &QQuickWindow::beforeRendering, this,
          [window]() { probeZeroCopyReadyAtRender(); }, Qt::DirectConnection);
  connect(window, &QQuickWindow::sceneGraphInvalidated,
          this, &PAGView::onSceneGraphInvalidated, Qt::QueuedConnection);
  connect(window, &QWindow::screenChanged, this, [this](QScreen* screen) {
    cancelPlayback();
    {
      QMutexLocker locker(&stateMutex_);
      ++windowEpoch_;
    }
    observeScreen(screen);
    maybeRestartWallpaperAfterCancel();
  });
  connect(window, &QWindow::visibleChanged, this, [this](bool visible) {
    if (active_) {
      cancelPlayback();
      {
        QMutexLocker locker(&stateMutex_);
        ++windowEpoch_;
      }
      qDebug().noquote().nospace()
          << "[PAG_DEBUG] event=window-visible-cancel this=" << this
          << " visible=" << visible << " gen=" << generation_
          << " windowEpoch=" << windowEpoch_;
      maybeRestartWallpaperAfterCancel();
    }
  });
  observeScreen(window->screen());
}

void PAGView::restartStabilityTimer() {
  QMutexLocker locker(&stateMutex_);
  if (!active_) {
    return;
  }
  // 热路径仍通过事件循环异步推进，避免在状态更新过程中重入准备流程
  stabilityTimer_.start(skipStabilityDelay_ && windowIsReady() ? 0 : 200);
}

// PAG 首帧依赖场景图窗口、尺寸、主屏和 DPR 同时就绪
/*
 * 首帧准备必须等待 Qt Quick 窗口真正 exposed，且 item 尺寸、主屏状态和 DPR 均有效
 * 这些条件任一变化都会使当前会话重新进入稳定等待或被取消
 */
bool PAGView::windowIsReady() const {
  QMutexLocker locker(&stateMutex_);
  return staticReady_ && primary_ && observedWindow_ && observedWindow_->isExposed() &&
         width() > 0.0 && height() > 0.0 && observedWindow_->devicePixelRatio() > 0.0;
}

// 稳定窗口后才向工作线程发送解码请求，避免登录阶段反复重建资源
void PAGView::prepareAfterStableWindow() {
  QMutexLocker locker(&stateMutex_);
  if (!active_) {
    return;
  }
  if (!windowIsReady()) {
    skipStabilityDelay_ = false;
    stabilityTimer_.start(200);
    return;
  }
  const auto dpr = observedWindow_->devicePixelRatio();
  const int pixelWidth = qRound(width() * dpr);
  const int pixelHeight = qRound(height() * dpr);
  loadingStarted_ = true;
#if 0  // 会话级日志（默认关闭；需要跟踪加载尺寸/时序可切换 #if 1）
  qDebug().noquote().nospace()
      << "[PAG_SESSION] event=load-dispatch gen=" << generation_
      << " windowEpoch=" << windowEpoch_
      << " visibilityEpoch=" << visibilityEpoch_
      << " size=" << pixelWidth << "x" << pixelHeight
      << " stabilityDelayMs=" << (skipStabilityDelay_ ? 0 : stabilityTimer_.interval());
#endif
  QMetaObject::invokeMethod(renderThread_.get(),
                            wallpaperActive_ ? "loadVideoLoop" : "loadAndPrepare",
                            Qt::QueuedConnection,
                            Q_ARG(QString, filePath_),
                            Q_ARG(int, scaleModeForOption(option_)),
                            Q_ARG(int, pixelWidth), Q_ARG(int, pixelHeight),
                            Q_ARG(qulonglong, generation_), Q_ARG(qulonglong, windowEpoch_),
                            Q_ARG(qulonglong, visibilityEpoch_));
}

bool PAGView::ticketMatches(const PAGFrameTicket& ticket) const {
  QMutexLocker locker(&stateMutex_);
  return active_ && !cancelling_ && ticket.generation == generation_ &&
         ticket.windowEpoch == windowEpoch_ && ticket.visibilityEpoch == visibilityEpoch_;
}

// 工作线程只负责产生票据，主线程在场景图更新时负责附着纹理
/*
 * 工作线程产生票据后只把它放入 pending 队列，不直接操作场景图
 * 如果票据的 generation、windowEpoch 或 visibilityEpoch 已过期，说明它属于旧会话，必须丢弃
 */
void PAGView::onFrameFlushed(const PAGFrameTicket& ticket) {
  QMutexLocker locker(&stateMutex_);
  if (!ticketMatches(ticket) || pendingTicketValid_ || attachedTicketValid_) {
#if 0  // 异常路径日志（默认关闭；排查丢弃帧/取消竞态时可切换 #if 1）
    qDebug().noquote().nospace()
        << "[PAG_CANCEL_TRACE] event=frame-flushed-discarded"
        << " ticketGen=" << ticket.generation
        << " ticketWindowEpoch=" << ticket.windowEpoch
        << " ticketVisibilityEpoch=" << ticket.visibilityEpoch
        << " serial=" << ticket.serial
        << " active=" << active_
        << " cancelling=" << cancelling_
        << " pending=" << pendingTicketValid_
        << " attached=" << attachedTicketValid_
        << " gen=" << generation_
        << " windowEpoch=" << windowEpoch_
        << " visibilityEpoch=" << visibilityEpoch_;
#endif
    return;
  }
  pendingTicket_ = ticket;
  pendingTicketValid_ = true;
  update();
}

// 按 pending → attached → swapped 顺序推进一帧，避免 QQuickWindow 合并更新导致跳帧
/*
 * 场景图线程在这里完成 pending → attached：将工作线程生成的纹理挂到 QSG 节点
 * 只有 attached 票据在 frameSwapped 中确认后，才会通知工作线程继续下一帧
 */
QSGNode* PAGView::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) {
  QMutexLocker locker(&stateMutex_);
  auto* node = static_cast<QSGImageNode*>(oldNode);
  // 设备创建完成会触发一次无节点更新，仅在此时重试待处理首帧
  if (!node && active_ && !cancelling_ && loadingStarted_ &&
      !pendingTicketValid_ && !attachedTicketValid_) {
    QMetaObject::invokeMethod(renderThread_.get(), "retryPendingPrepare",
                              Qt::QueuedConnection,
                              Q_ARG(qulonglong, generation_),
                              Q_ARG(qulonglong, windowEpoch_),
                              Q_ARG(qulonglong, visibilityEpoch_));
  }
  // 锁屏/解锁期间（cancelling_ 或 waitingForDetach_）保留 QSGImageNode：
  // 返回 nullptr 会让 Qt 销毁节点，其成员材质对象随之释放，而 QSGBatchRenderer
  // 跨帧 batch 缓存仍可能引用该材质，快速锁屏解锁时发生 UAF（compare 崩溃）。
  // 仅在真正停止（!active_ && !cancelling_ && !waitingForDetach_）时移除节点。
  if (!active_ && !cancelling_ && !waitingForDetach_) {
    nodeAttached_ = false;
    if (oldNode) {
      detachPending_ = true;
    }
    return nullptr;
  }
  if (!pendingTicketValid_) {
    return node;
  }
  auto* texture = drawable_->getTexture();
  if (!texture || !window()) {
    return node;
  }
  if (!node) {
    node = window()->createImageNode();
  }
  node->setTexture(texture);
  node->setOwnsTexture(false);
  node->setRect(boundingRect());
  node->markDirty(QSGNode::DirtyMaterial | QSGNode::DirtyGeometry);
  attachedTicket_ = pendingTicket_;
  attachedTicketValid_ = true;
  pendingTicketValid_ = false;
  nodeAttached_ = true;
#if 0  // PAG 逐帧调试日志（默认关闭，排查问题时可切换 #if 1 临时启用）
  qDebug().noquote().nospace()
      << "[PAG_FRAME] phase=attached gen=" << attachedTicket_.generation
      << " windowEpoch=" << attachedTicket_.windowEpoch
      << " visibilityEpoch=" << attachedTicket_.visibilityEpoch
      << " serial=" << attachedTicket_.serial
      << " ticketPhase="
      << (attachedTicket_.phase == PAGFrameTicket::PrepareHidden ? "prepare" : "visible")
      << " frame=" << attachedTicket_.frame
      << " itemSize=" << boundingRect().width() << "x" << boundingRect().height();
  qDebug().noquote().nospace()
      << "[PAG_SIZE_TRACE] event=frame-attached"
      << " itemSize=" << boundingRect().width() << "x" << boundingRect().height()
      << " windowSize=" << window()->width() << "x" << window()->height()
      << " textureSize=" << texture->textureSize().width() << "x" << texture->textureSize().height()
      << " dpr=" << window()->devicePixelRatio();
#endif
  return node;
}

// frameSwapped 是当前帧真正可见的确认点，随后才允许工作线程生成下一帧
/*
 * frameSwapped 表示这一帧已经被 Qt Quick 提交到显示管线
 * 因此它是 produced、attached、swapped 三阶段中唯一允许推进下一帧的确认点
 */
void PAGView::onFrameSwapped() {
  PAGFrameTicket presentedTicket;
  bool presentTicket = false;
  bool notifyDetach = false;
  bool resumePreparation = false;
  qulonglong detachGeneration = 0;
  qulonglong detachWindowEpoch = 0;
  qulonglong detachVisibilityEpoch = 0;
  qulonglong resumeGeneration = 0;
  qulonglong resumeWindowEpoch = 0;
  qulonglong resumeVisibilityEpoch = 0;

  {
    // 此槽函数运行在场景图线程中，此处仅在互斥锁保护下获取状态
    // GUI 定时器、QML 可见信号和项目状态切换均由排队的 GUI 槽函数处理
    QMutexLocker locker(&stateMutex_);
    if (detachPending_ && !nodeAttached_) {
      detachPending_ = false;
      notifyDetach = true;
      detachGeneration = cancelGeneration_;
      detachWindowEpoch = cancelWindowEpoch_;
      detachVisibilityEpoch = cancelVisibilityEpoch_;
      if (waitingForDetach_) {
        waitingForDetach_ = false;
        resumePreparation = active_;
        resumeGeneration = generation_;
        resumeWindowEpoch = windowEpoch_;
        resumeVisibilityEpoch = visibilityEpoch_;
      }
    }
    if (attachedTicketValid_) {
      const auto ticket = attachedTicket_;
      attachedTicketValid_ = false;
      if (active_ && !cancelling_ && ticket.generation == generation_ &&
          ticket.windowEpoch == windowEpoch_ &&
          ticket.visibilityEpoch == visibilityEpoch_) {
        presentedTicket = ticket;
        presentTicket = true;
      }
    }
  }

#if 0  // PAG 逐帧调试日志（默认关闭，排查问题时可切换 #if 1 临时启用）
  if (presentTicket) {
    qDebug().noquote().nospace()
        << "[PAG_FRAME] phase=swapped gen=" << presentedTicket.generation
        << " windowEpoch=" << presentedTicket.windowEpoch
        << " visibilityEpoch=" << presentedTicket.visibilityEpoch
        << " serial=" << presentedTicket.serial
        << " ticketPhase="
        << (presentedTicket.phase == PAGFrameTicket::PrepareHidden ? "prepare" : "visible")
        << " frame=" << presentedTicket.frame;
  }
#endif
  if (notifyDetach) {
    QMetaObject::invokeMethod(renderThread_.get(), "nodeDetached", Qt::QueuedConnection,
                              Q_ARG(qulonglong, detachGeneration),
                              Q_ARG(qulonglong, detachWindowEpoch),
                              Q_ARG(qulonglong, detachVisibilityEpoch));
  }
  if (resumePreparation) {
    QMetaObject::invokeMethod(this, "resumeAfterDetach", Qt::QueuedConnection,
                              Q_ARG(qulonglong, resumeGeneration),
                              Q_ARG(qulonglong, resumeWindowEpoch),
                              Q_ARG(qulonglong, resumeVisibilityEpoch));
  }
  if (presentTicket) {
    QMetaObject::invokeMethod(renderThread_.get(), "framePresented", Qt::QueuedConnection,
                              Q_ARG(pag::PAGFrameTicket, presentedTicket));
  }
}

void PAGView::resumeAfterDetach(qulonglong generation, qulonglong windowEpoch,
                                qulonglong visibilityEpoch) {
  QMutexLocker locker(&stateMutex_);
  if (!active_ || cancelling_ || waitingForDetach_ || generation != generation_ ||
      windowEpoch != windowEpoch_ || visibilityEpoch != visibilityEpoch_) {
    return;
  }
  restartStabilityTimer();
}

// 只有节点已脱离场景图且渲染线程确认后，才能释放 PAG 对象
void PAGView::onWorkerSafeToUnload(qulonglong generation, qulonglong windowEpoch,
                                   qulonglong visibilityEpoch) {
  Q_UNUSED(windowEpoch)
  Q_UNUSED(visibilityEpoch)
  QMutexLocker locker(&stateMutex_);
  if (active_) {
    return;
  }
  // 新请求可能在等待旧节点分离时已经被取消。此时工作线程已清空且 QSG 节点已移除
  // 因此由当前代次负责完成卸载
  Q_EMIT safeToUnload(generation_ ? generation_ : generation);
}

void PAGView::onHiddenFramePresented(const PAGFrameTicket& ticket) {
  QMutexLocker locker(&stateMutex_);
  if (!ticketMatches(ticket) || ticket.phase != PAGFrameTicket::PrepareHidden) {
    return;
  }
  if (wallpaperActive_ && !paused_) {
    // mp4 壁纸未锁定：首帧就绪即自动进入可见播放（prepared 回调检查 paused 态，
    // 锁定中不启动可见播放，等价 pag 解锁瞬间起播语义；prepare 中解锁亦由此路径启动）
    const auto generation = generation_;
    beginVisiblePlayback(generation);
  }
  Q_EMIT prepared(generation_);
}

void PAGView::onPlaybackFinished(const PAGFrameTicket& ticket) {
  QMutexLocker locker(&stateMutex_);
  if (!ticketMatches(ticket) || ticket.phase != PAGFrameTicket::VisiblePlayback) {
    return;
  }
  const auto generation = generation_;
  cancelPlayback();
  Q_EMIT finished(generation);
}

void PAGView::onLoadFailed(qulonglong generation, qulonglong windowEpoch,
                           qulonglong visibilityEpoch, const QString& reason) {
  QMutexLocker locker(&stateMutex_);
  if (!active_ || generation != generation_ || windowEpoch != windowEpoch_ ||
      visibilityEpoch != visibilityEpoch_) {
    return;
  }
  cancelPlayback();
  Q_EMIT failed(generation, reason);
}

/*
 * windowEpoch 专门表示窗口或场景图生命周期变化；窗口切换、屏幕变化和尺寸变化都会递增它
 * 旧窗口回调即使晚到，也无法通过票据校验影响新窗口
 */
void PAGView::bumpWindowEpochAndCancel() {
  QMutexLocker locker(&stateMutex_);
  // 节点保留方案下 detach 在 cancelPlayback 内同步完成，此处无需再计算
  // mustDetachOldNode / waitingForDetach_。
  cancelPlayback();
  ++windowEpoch_;
}

void PAGView::observeScreen(QScreen* screen) {
  QMutexLocker locker(&stateMutex_);
  if (observedScreen_) {
    disconnect(observedScreen_, nullptr, this, nullptr);
  }
  observedScreen_ = screen;
  qDebug().noquote().nospace()
      << "[PAG_DEBUG] event=screen-observed this=" << this
      << " screen=" << (screen ? screen->name() : "null")
      << " dpr=" << (screen ? screen->devicePixelRatio() : -1.0)
      << " geom=" << (screen ? screen->geometry().width() : 0) << "x"
      << (screen ? screen->geometry().height() : 0);
  if (!screen) {
    return;
  }
  auto metricChanged = [this](qreal) {
    qDebug().noquote().nospace()
        << "[PAG_DEBUG] event=metric-changed(dpi) this=" << this
        << " active=" << active_ << " loading=" << loadingStarted_
        << " screenDpr=" << (observedScreen_ ? observedScreen_->devicePixelRatio() : -1.0)
        << " winDpr=" << (observedWindow_ ? observedWindow_->devicePixelRatio() : -1.0)
        << " item=" << width() << "x" << height();
    if (!active_) {
      return;
    }
    if (loadingStarted_) {
      cancelPlayback();
      {
        QMutexLocker locker(&stateMutex_);
        ++windowEpoch_;
      }
      maybeRestartWallpaperAfterCancel();
    } else {
      skipStabilityDelay_ = false;
      ++windowEpoch_;
      restartStabilityTimer();
    }
  };
  connect(screen, &QScreen::logicalDotsPerInchChanged, this, metricChanged);
  connect(screen, &QScreen::physicalDotsPerInchChanged, this, metricChanged);
  connect(screen, &QScreen::geometryChanged, this, [this](const QRect& g) {
    qDebug().noquote().nospace()
        << "[PAG_DEBUG] event=screen-geometry-changed geom="
        << g.width() << "x" << g.height() << "@" << g.x() << "," << g.y()
        << " active=" << active_ << " loading=" << loadingStarted_;
  });
}

void PAGView::notifyNodeDetached() {
  QMutexLocker locker(&stateMutex_);
  QMetaObject::invokeMethod(renderThread_.get(), "nodeDetached", Qt::QueuedConnection,
                            Q_ARG(qulonglong, cancelGeneration_),
                            Q_ARG(qulonglong, cancelWindowEpoch_),
                            Q_ARG(qulonglong, cancelVisibilityEpoch_));
}

void PAGView::onSceneGraphInvalidated() {
  QMutexLocker locker(&stateMutex_);
#if 0  // 诊断日志：场景图失效时机与节点状态（排查生命周期问题时启用）
  qDebug().noquote().nospace()
      << "[PAG_GL] event=scenegraph-invalidated this=" << this
      << " thread=" << QThread::currentThread()
      << " nodeAttached=" << nodeAttached_
      << " waitingForDetach=" << waitingForDetach_;
#endif
  nodeAttached_ = false;
  detachPending_ = false;
  notifyNodeDetached();
  if (waitingForDetach_) {
    waitingForDetach_ = false;
    restartStabilityTimer();
  }
}

void PAGView::releaseResources() {
  QMutexLocker locker(&stateMutex_);
#if 0  // 诊断日志：窗口/上下文销毁时释放资源（排查生命周期问题时启用）
  qDebug().noquote().nospace()
      << "[PAG_GL] event=release-resources this=" << this
      << " thread=" << QThread::currentThread()
      << " nodeAttached=" << nodeAttached_
      << " waitingForDetach=" << waitingForDetach_;
#endif
  nodeAttached_ = false;
  notifyNodeDetached();
  if (waitingForDetach_) {
    waitingForDetach_ = false;
    restartStabilityTimer();
  }
}

bool PAGView::eventFilter(QObject* watched, QEvent* event) {
  QMutexLocker locker(&stateMutex_);
  if (watched == observedWindow_ && event->type() == QEvent::Expose && active_) {
    if (loadingStarted_ && !observedWindow_->isExposed()) {
      cancelPlayback();
      {
        QMutexLocker locker(&stateMutex_);
        ++windowEpoch_;
      }
      maybeRestartWallpaperAfterCancel();
    } else if (!loadingStarted_) {
      restartStabilityTimer();
    }
  }
  return QQuickItem::eventFilter(watched, event);
}

}  // namespace pag
