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

  renderThread_->start();
  renderThread_->setPriority(QThread::NormalPriority);
  stabilityTimer_.setSingleShot(true);
  stabilityTimer_.setInterval(200);
  connect(&stabilityTimer_, &QTimer::timeout, this, &PAGView::prepareAfterStableWindow);
  connect(this, &QQuickItem::windowChanged, this, &PAGView::onWindowChanged);
}

PAGView::~PAGView() {
  active_ = false;
  pendingTicketValid_ = false;
  attachedTicketValid_ = false;
  QMetaObject::invokeMethod(renderThread_.get(), "shutDown", Qt::QueuedConnection);
  renderThread_->wait();
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
  const bool mustDetachOldNode = nodeAttached_ || detachPending_;
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
    waitingForDetach_ = mustDetachOldNode;
    if (!mustDetachOldNode) {
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
  waitingForDetach_ = mustDetachOldNode;
  ++visibilityEpoch_;
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
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
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
  qDebug().noquote().nospace()
      << "[PAG_SESSION] event=resume-pending-frame gen=" << generation_
      << " pending=" << pendingTicketValid_
      << " attached=" << attachedTicketValid_;
#endif
  update();
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
  detachPending_ = nodeAttached_;
  Q_EMIT activeChanged(false);
  update();
  QMetaObject::invokeMethod(renderThread_.get(), "cancel", Qt::QueuedConnection,
                            Q_ARG(qulonglong, oldGeneration), Q_ARG(qulonglong, oldWindowEpoch),
                            Q_ARG(qulonglong, oldVisibilityEpoch));
  if (!nodeAttached_) {
    notifyNodeDetached();
  }
  if (!workerLoadStarted && !nodeAttached_ && !waitingForDetach_) {
    QMetaObject::invokeMethod(renderThread_.get(), "cancelEmptySession", Qt::QueuedConnection,
                              Q_ARG(qulonglong, oldGeneration),
                              Q_ARG(qulonglong, oldWindowEpoch),
                              Q_ARG(qulonglong, oldVisibilityEpoch));
  }
}

// 窗口尺寸变化会使旧渲染结果失效，递增 windowEpoch 隔离异步回调
void PAGView::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry) {
  QMutexLocker locker(&stateMutex_);
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
    } else {
      skipStabilityDelay_ = false;
      ++windowEpoch_;
      restartStabilityTimer();
    }
  }
}

void PAGView::onWindowChanged(QQuickWindow* window) {
  QMutexLocker locker(&stateMutex_);
  if (active_) {
    cancelPlayback();
  }
  if (observedWindow_) {
    observedWindow_->removeEventFilter(this);
    disconnect(observedWindow_, nullptr, this, nullptr);
  }
  observeScreen(nullptr);
  observedWindow_ = window;
  ++windowEpoch_;
  if (!window) {
    cancelPlayback();
    return;
  }
  window->installEventFilter(this);
  // 此处有意使用 DirectConnection：交换操作只能消费紧邻的上一次 updatePaintNode
  // 所挂接的帧凭据，不能消费 GUI 线程队列中无关交换对应的凭据
  connect(window, &QQuickWindow::frameSwapped, this, &PAGView::onFrameSwapped,
          Qt::DirectConnection);
  connect(window, &QQuickWindow::sceneGraphInvalidated,
          this, &PAGView::onSceneGraphInvalidated, Qt::QueuedConnection);
  connect(window, &QWindow::screenChanged, this, [this](QScreen* screen) {
    cancelPlayback();
    {
      QMutexLocker locker(&stateMutex_);
      ++windowEpoch_;
    }
    observeScreen(screen);
  });
  connect(window, &QWindow::visibleChanged, this, [this](bool) {
    if (active_) {
      cancelPlayback();
      {
        QMutexLocker locker(&stateMutex_);
        ++windowEpoch_;
      }
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
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
  qDebug().noquote().nospace()
      << "[PAG_SESSION] event=load-dispatch gen=" << generation_
      << " windowEpoch=" << windowEpoch_
      << " visibilityEpoch=" << visibilityEpoch_
      << " size=" << pixelWidth << "x" << pixelHeight
      << " stabilityDelayMs=" << (skipStabilityDelay_ ? 0 : stabilityTimer_.interval());
#endif
  QMetaObject::invokeMethod(renderThread_.get(), "loadAndPrepare", Qt::QueuedConnection,
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
#if 0
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
  if (!active_ || cancelling_ || waitingForDetach_) {
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
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
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

#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
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
  if (ticketMatches(ticket) && ticket.phase == PAGFrameTicket::PrepareHidden) {
    Q_EMIT prepared(generation_);
  }
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
  cancelPlayback();
  ++windowEpoch_;
}

void PAGView::observeScreen(QScreen* screen) {
  QMutexLocker locker(&stateMutex_);
  if (observedScreen_) {
    disconnect(observedScreen_, nullptr, this, nullptr);
  }
  observedScreen_ = screen;
  if (!screen) {
    return;
  }
  auto metricChanged = [this](qreal) {
    if (!active_) {
      return;
    }
    if (loadingStarted_) {
      cancelPlayback();
      {
        QMutexLocker locker(&stateMutex_);
        ++windowEpoch_;
      }
    } else {
      skipStabilityDelay_ = false;
      ++windowEpoch_;
      restartStabilityTimer();
    }
  };
  connect(screen, &QScreen::logicalDotsPerInchChanged, this, metricChanged);
  connect(screen, &QScreen::physicalDotsPerInchChanged, this, metricChanged);
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
    } else if (!loadingStarted_) {
      restartStabilityTimer();
    }
  }
  return QQuickItem::eventFilter(watched, event);
}

}  // namespace pag
