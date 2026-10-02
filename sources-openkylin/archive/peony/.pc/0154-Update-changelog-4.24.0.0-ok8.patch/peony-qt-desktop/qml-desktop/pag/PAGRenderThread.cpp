/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making libpag available.
//
//  Copyright (C) 2021 Tencent. All rights reserved.
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

#include "PAGRenderThread.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QUrl>
#include <QtMath>
#include <limits>
#include "GPUDrawable.h"

namespace pag {

PAGRenderThread::PAGRenderThread(const std::shared_ptr<GPUDrawable>& drawable)
    : drawable_(drawable) {
}

bool PAGRenderThread::matches(qulonglong generation, qulonglong windowEpoch,
                              qulonglong visibilityEpoch) const {
  return generation == generation_ && windowEpoch == windowEpoch_ &&
         visibilityEpoch == visibilityEpoch_;
}

// 工作线程独占 PAGPlayer/PAGFile，主线程只通过信号和帧票据与其交互
/*
 * 加载阶段只在工作线程创建 PAGFile、PAGPlayer 和 GPU surface
 * 主线程不会直接触碰这些对象，避免解码、渲染与 Qt Quick 场景图并发访问
 */
void PAGRenderThread::loadAndPrepare(const QString& filePath, int scaleMode, int pixelWidth,
                                     int pixelHeight, qulonglong generation,
                                     qulonglong windowEpoch, qulonglong visibilityEpoch) {
  unload();
  generation_ = generation;
  windowEpoch_ = windowEpoch;
  visibilityEpoch_ = visibilityEpoch;
  cancelling_ = false;
  nodeDetached_ = false;
  safeUnloadEmitted_ = false;
  prepareFramePending_ = false;
  nextSerial_ = 0;

  if (filePath.isEmpty() || pixelWidth <= 0 || pixelHeight <= 0) {
    Q_EMIT loadFailed(generation_, windowEpoch_, visibilityEpoch_,
                      QStringLiteral("invalid path or render size"));
    return;
  }

  const auto localPath = filePath.startsWith(QLatin1String("file://"))
                             ? QUrl(filePath).toLocalFile()
                             : filePath;
  auto bytes = ByteData::FromPath(localPath.toLocal8Bit().constData());
  if (!bytes) {
    Q_EMIT loadFailed(generation_, windowEpoch_, visibilityEpoch_,
                      QStringLiteral("failed to read PAG file"));
    return;
  }
  file_ = PAGFile::Load(bytes->data(), bytes->length(), localPath.toLocal8Bit().constData());
  if (!file_) {
    Q_EMIT loadFailed(generation_, windowEpoch_, visibilityEpoch_,
                      QStringLiteral("failed to decode PAG file"));
    return;
  }

  player_.reset(new PAGPlayer());
  player_->setSurface(PAGSurface::MakeFrom(drawable_));
  player_->setComposition(file_);
  player_->setScaleMode(static_cast<PAGScaleMode>(scaleMode));
  player_->getSurface()->updateSize();

  const auto fps = file_->frameRate() > 0.0f ? file_->frameRate() : 30.0f;
  totalFrames_ = qMax(1, static_cast<int>(qRound64(file_->duration() * fps / 1000000.0)));
  framePeriodNs_ = qMax<qint64>(1, qRound64(1000000000.0 / fps));
  nextVisibleFrame_ = 0;
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
  qDebug().noquote().nospace()
      << "[PAG_SESSION] event=prepare gen=" << generation_
      << " windowEpoch=" << windowEpoch_
      << " visibilityEpoch=" << visibilityEpoch_
      << " path=" << localPath
      << " size=" << pixelWidth << "x" << pixelHeight
      << " fps=" << fps
      << " totalFrames=" << totalFrames_
      << " intervalNs=" << framePeriodNs_;
#endif
  submitFrame(0, PAGFrameTicket::PrepareHidden);
}

// 每次提交只允许一个 in-flight 票据，必须收到 swapped 才能继续
/*
 * submitFrame 只生成一个带完整代次信息的帧票据
 * ticketInFlight_ 保证上一帧尚未 swapped 时不会提交下一帧，从而保持帧序连续
 */
void PAGRenderThread::submitFrame(int frame, PAGFrameTicket::Phase phase) {
  if (!player_ || !file_ || cancelling_ || ticketInFlight_ ||
      (phase == PAGFrameTicket::VisiblePlayback && !visiblePlaybackStarted_)) {
    return;
  }
  // 新建 QGLWindow 的设备由场景图异步创建，未就绪时不能生成帧凭据
  if (phase == PAGFrameTicket::PrepareHidden && !drawable_->getDevice()) {
    prepareFramePending_ = true;
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
    qDebug().noquote().nospace()
        << "[PAG_SESSION] event=prepare-device-pending gen=" << generation_
        << " windowEpoch=" << windowEpoch_
        << " visibilityEpoch=" << visibilityEpoch_;
#endif
    return;
  }
  const double progress =
      frame >= totalFrames_ - 1 ? 1.0 : (static_cast<double>(frame) + 0.1) / totalFrames_;
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
  QElapsedTimer renderTimer;
  renderTimer.start();
#endif
  player_->setProgress(progress);
  const bool changed = player_->flush();

  prepareFramePending_ = false;
  inFlight_.generation = generation_;
  inFlight_.windowEpoch = windowEpoch_;
  inFlight_.serial = ++nextSerial_;
  inFlight_.frame = frame;
  inFlight_.visibilityEpoch = visibilityEpoch_;
  inFlight_.phase = phase;
  ticketInFlight_ = true;
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
  qDebug().noquote().nospace()
      << "[PAG_FRAME] phase=produced gen=" << inFlight_.generation
      << " windowEpoch=" << inFlight_.windowEpoch
      << " visibilityEpoch=" << inFlight_.visibilityEpoch
      << " serial=" << inFlight_.serial
      << " ticketPhase="
      << (phase == PAGFrameTicket::PrepareHidden ? "prepare" : "visible")
      << " frame=" << frame << "/" << totalFrames_
      << " progress=" << progress
      << " changed=" << changed
      << " renderMs=" << renderTimer.elapsed();
#endif
  Q_EMIT frameFlushed(inFlight_);
}

// 场景图设备尚未创建时暂存首帧，设备就绪后仅重试一次首帧准备
void PAGRenderThread::retryPendingPrepare(qulonglong generation, qulonglong windowEpoch,
                                          qulonglong visibilityEpoch) {
  if (!prepareFramePending_ || !matches(generation, windowEpoch, visibilityEpoch) ||
      cancelling_ || ticketInFlight_ || !player_ || !file_) {
    return;
  }
  prepareFramePending_ = false;
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
  qDebug().noquote().nospace()
      << "[PAG_SESSION] event=prepare-device-retry gen=" << generation_
      << " windowEpoch=" << windowEpoch_
      << " visibilityEpoch=" << visibilityEpoch_;
#endif
  submitFrame(0, PAGFrameTicket::PrepareHidden);
}

void PAGRenderThread::startVisiblePlayback(qulonglong generation, qulonglong windowEpoch,
                                           qulonglong visibilityEpoch) {
  if (!matches(generation, windowEpoch, visibilityEpoch) || cancelling_ || ticketInFlight_) {
    return;
  }
  invalidateFrameSchedule();
  visiblePlaybackStarted_ = true;
  playbackClock_.start();
  nextVisibleFrame_ = 0;
  submitFrame(0, PAGFrameTicket::VisiblePlayback);
}

// 主线程确认票据已交换后，工作线程才推进可见帧序号
/*
 * framePresented 来自主线程的 swapped 回调
 * PrepareHidden 票据只表示首帧已准备完成，VisiblePlayback 票据才会推进下一帧或结束播放
 */
void PAGRenderThread::framePresented(const PAGFrameTicket& ticket) {
  if (!ticketInFlight_ || !(ticket == inFlight_)) {
    return;
  }
  ticketInFlight_ = false;
  if (cancelling_) {
    unloadIfDetached();
    return;
  }
  if (ticket.phase == PAGFrameTicket::PrepareHidden) {
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
    qDebug().noquote().nospace()
        << "[PAG_SESSION] event=prepare-presented gen=" << generation_
        << " serial=" << ticket.serial;
#endif
    Q_EMIT hiddenFramePresented(ticket);
    return;
  }
  if (ticket.frame >= totalFrames_ - 1) {
    invalidateFrameSchedule();
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
    qDebug().noquote().nospace()
        << "[PAG_SUMMARY] event=completed gen=" << generation_
        << " expectedVisibleFrames=" << totalFrames_
        << " lastSerial=" << ticket.serial
        << " lastFrame=" << ticket.frame;
#endif
    Q_EMIT playbackFinished(ticket);
    return;
  }
  nextVisibleFrame_ = ticket.frame + 1;
  scheduleNextVisibleFrame();
}

// 使用单调时钟按原始帧间隔调度，避免渲染耗时造成累计漂移
void PAGRenderThread::scheduleNextVisibleFrame() {
  if (!visiblePlaybackStarted_ || !playbackClock_.isValid() || cancelling_ ||
      ticketInFlight_ || nextVisibleFrame_ >= totalFrames_) {
    return;
  }

  const auto token = ++scheduleEpoch_;
  const auto generation = generation_;
  const auto windowEpoch = windowEpoch_;
  const auto visibilityEpoch = visibilityEpoch_;
  const auto frame = nextVisibleFrame_;
  const auto deadlineNs = framePeriodNs_ * static_cast<qint64>(frame);
  const auto remainingNs = deadlineNs - playbackClock_.nsecsElapsed();
  if (remainingNs <= 0) {
    submitFrame(frame, PAGFrameTicket::VisiblePlayback);
    return;
  }

  const auto delayMs64 = qMax<qint64>(1, (remainingNs + 999999) / 1000000);
  const auto delayMs = static_cast<int>(
      qMin<qint64>(delayMs64, std::numeric_limits<int>::max()));
  QTimer::singleShot(delayMs, Qt::PreciseTimer, this,
                     [this, token, generation, windowEpoch, visibilityEpoch, frame]() {
    if (token != scheduleEpoch_ ||
        !matches(generation, windowEpoch, visibilityEpoch) ||
        !visiblePlaybackStarted_ || cancelling_ || ticketInFlight_ ||
        frame != nextVisibleFrame_) {
      return;
    }
    // 重新读取单调时钟，避免定时器提前唤醒时过早提交帧
    scheduleNextVisibleFrame();
  });
}

void PAGRenderThread::invalidateFrameSchedule() {
  ++scheduleEpoch_;
  visiblePlaybackStarted_ = false;
  playbackClock_.invalidate();
}

// 取消会递增调度代次，使已经排队的定时器回调自动失效
/*
 * 取消时使当前会话停止接收新帧，并递增调度代次使排队定时器失效
 * 已经产生但尚未交换的票据仍会安全回收，不会覆盖后续会话
 */
void PAGRenderThread::cancel(qulonglong generation, qulonglong windowEpoch,
                             qulonglong visibilityEpoch) {
  if (!matches(generation, windowEpoch, visibilityEpoch)) {
    return;
  }
  cancelling_ = true;
  prepareFramePending_ = false;
  invalidateFrameSchedule();
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
  qDebug().noquote().nospace()
      << "[PAG_SUMMARY] event=cancel gen=" << generation_
      << " windowEpoch=" << windowEpoch_
      << " visibilityEpoch=" << visibilityEpoch_
      << " inFlight=" << ticketInFlight_
      << " lastSerial=" << nextSerial_;
#endif
  Q_EMIT cancelAcknowledged(generation_, windowEpoch_, visibilityEpoch_);
  unloadIfDetached();
}

void PAGRenderThread::cancelEmptySession(qulonglong generation, qulonglong windowEpoch,
                                         qulonglong visibilityEpoch) {
  if (player_ || file_ || ticketInFlight_) {
    return;
  }
  if (matches(generation, windowEpoch, visibilityEpoch) && safeUnloadEmitted_) {
    return;
  }
  generation_ = generation;
  windowEpoch_ = windowEpoch;
  visibilityEpoch_ = visibilityEpoch;
  safeUnloadEmitted_ = false;
  cancelling_ = true;
  prepareFramePending_ = false;
  nodeDetached_ = true;
  invalidateFrameSchedule();
  Q_EMIT cancelAcknowledged(generation_, windowEpoch_, visibilityEpoch_);
  unloadIfDetached();
}

void PAGRenderThread::nodeDetached(qulonglong generation, qulonglong windowEpoch,
                                   qulonglong visibilityEpoch) {
  if (!matches(generation, windowEpoch, visibilityEpoch)) {
    return;
  }
  nodeDetached_ = true;
  unloadIfDetached();
}

// 延迟卸载保证取消前已经提交的 GPU 工作完成后再释放资源
void PAGRenderThread::unloadIfDetached() {
  if (cancelling_ && nodeDetached_ && !safeUnloadEmitted_) {
    // 取消前提交的一次渲染仍可能到达；PAGView 会丢弃该渲染
    // 释放工作线程持有的 PAG 对象不需要等待其确认
    const auto generation = generation_;
    const auto windowEpoch = windowEpoch_;
    const auto visibilityEpoch = visibilityEpoch_;
    safeUnloadEmitted_ = true;
    unload();
#if 0  // PAG 逐帧调试日志，排查问题时可临时启用
    qDebug().noquote().nospace()
        << "[PAG_SESSION] event=safe-unload gen=" << generation
        << " windowEpoch=" << windowEpoch
        << " visibilityEpoch=" << visibilityEpoch;
#endif
    Q_EMIT safeToUnloadWorker(generation, windowEpoch, visibilityEpoch);
  }
}

void PAGRenderThread::unload() {
  invalidateFrameSchedule();
  prepareFramePending_ = false;
  ticketInFlight_ = false;
  player_.reset();
  file_.reset();
  totalFrames_ = 0;
}

void PAGRenderThread::shutDown() {
  unload();
  if (QGuiApplication::instance()) {
    drawable_->moveToThread(QGuiApplication::instance()->thread());
    moveToThread(QGuiApplication::instance()->thread());
  }
  exit();
}

}  // namespace pag
