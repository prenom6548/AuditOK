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
#include <QOpenGLContext>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <malloc.h>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QFile>
#include <QUrl>
#include <QtMath>
#include <limits>
#include <mutex>
#include "GPUDrawable.h"

namespace {

// 多屏会话共享同一个 QSG 全局 GL context（QGLWindow::createDevice 直接复用
// OpenGLContextResource），而每个屏幕的 PAG 会话各自有工作线程并发 makeCurrent/
// flush。QOpenGLContext 非线程安全，并发 GL 操作会使飞腾等 GPU 驱动崩溃
// （实测 SIGSEGV 于 phytium_dri.so GLTexture::onRelease/purge 路径）。
// 全局串行化所有 PAG 工作线程的 flush 段，恢复单工作线程语义。
std::mutex g_pagGLLock;

// ISOBMFF 魔数探测（offset 4 == "ftyp"）：识别 AccountsService 复制的无扩展名 mp4
bool looksLikeMp4Magic(const QString& path) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) {
    return false;
  }
  const QByteArray head = f.read(16);
  return head.size() >= 12 && head.mid(4, 4) == QByteArrayLiteral("ftyp");
}


}  // namespace

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
  loopEnabled_ = false;
  loadCommon(filePath, scaleMode, pixelWidth, pixelHeight, generation, windowEpoch,
             visibilityEpoch);
}

// mp4 壁纸常驻会话入口：与 loadAndPrepare 行为一致，仅多 loop 语义（末帧 wrap）
void PAGRenderThread::loadVideoLoop(const QString& filePath, int scaleMode, int pixelWidth,
                                    int pixelHeight, qulonglong generation,
                                    qulonglong windowEpoch, qulonglong visibilityEpoch) {
  loopEnabled_ = true;
  loadCommon(filePath, scaleMode, pixelWidth, pixelHeight, generation, windowEpoch,
             visibilityEpoch);
}

void PAGRenderThread::loadCommon(const QString& filePath, int scaleMode, int pixelWidth,
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
  // 外部视频壁纸（.mp4）由 libpag 直接按路径加载，避免大文件整读进内存；
  // .pag 及静态壁纸路径保持原有加载逻辑完全不变。
  // 后缀判定之外再探测 ISOBMFF 魔数（offset 4 "ftyp"）：AccountsService 复制壁纸到
  // /var/lib/.../backgrounds/<user> 时会丢失扩展名，魔数保证这类副本仍走视频加载。
  const bool isMp4 = localPath.endsWith(QLatin1String(".mp4"), Qt::CaseInsensitive) ||
                     looksLikeMp4Magic(localPath);
  if (isMp4) {
    file_ = PAGFile::LoadFromVideo(localPath.toStdString());
    if (!file_) {
      Q_EMIT loadFailed(generation_, windowEpoch_, visibilityEpoch_,
                        QStringLiteral("failed to load video wallpaper"));
      return;
    }
  } else {
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
  }

  player_.reset(new PAGPlayer());
  surface_ = PAGSurface::MakeFrom(drawable_);
  player_->setSurface(surface_);
  player_->setComposition(file_);
  player_->setScaleMode(static_cast<PAGScaleMode>(scaleMode));
  player_->getSurface()->updateSize();

  const auto fps = file_->frameRate() > 0.0f ? file_->frameRate() : 30.0f;
  totalFrames_ = qMax(1, static_cast<int>(qRound64(file_->duration() * fps / 1000000.0)));
  framePeriodNs_ = qMax<qint64>(1, qRound64(1000000000.0 / fps));
  nextVisibleFrame_ = 0;
#if 0  // 会话级日志（默认关闭；需要跟踪工作线程加载会话时可切换 #if 1）
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
#if 0  // PAG 逐帧调试日志（默认关闭，排查问题时可切换 #if 1 临时启用）
  qDebug().noquote().nospace()
      << "[PAG_FRAME] stage=enter frame=" << frame;
#endif
  if (!player_ || !file_ || cancelling_ || ticketInFlight_ ||
      (phase == PAGFrameTicket::VisiblePlayback && !visiblePlaybackStarted_)) {
    return;
  }
  // 新建 QGLWindow 的设备由场景图异步创建，未就绪时不能生成帧凭据
  if (phase == PAGFrameTicket::PrepareHidden && !drawable_->getDevice()) {
    prepareFramePending_ = true;
#if 0  // 会话级日志（默认关闭；需要跟踪设备未就绪挂起时可切换 #if 1）
    qDebug().noquote().nospace()
        << "[PAG_SESSION] event=prepare-device-pending gen=" << generation_
        << " windowEpoch=" << windowEpoch_
        << " visibilityEpoch=" << visibilityEpoch_;
#endif
    return;
  }
  const double progress =
      frame >= totalFrames_ - 1 ? 1.0 : (static_cast<double>(frame) + 0.1) / totalFrames_;
  // 全局 GL 锁：多屏多工作线程并发 flush 同一共享 context 会崩溃驱动。
  // 锁只覆盖 GL 段（setProgress+flush），不跨越 frameFlushed 事件投递，避免死锁。
  std::lock_guard<std::mutex> glLock(g_pagGLLock);
#if 0  // PAG 逐帧调试日志（默认关闭，排查问题时可切换 #if 1 临时启用）
  QElapsedTimer renderTimer;
  renderTimer.start();
#endif
  player_->setProgress(progress);
#if 0  // PAG 逐帧调试日志（默认关闭，排查问题时可切换 #if 1 临时启用）
  qDebug().noquote().nospace()
      << "[PAG_FRAME] stage=preflush frame=" << frame
      << " progress=" << progress;
#endif
  const bool changed = player_->flush();
  Q_UNUSED(changed)  // 逐帧日志默认关闭，保留变量以匹配调试期签名；日后启用 #if 1 日志时继续使用
#if 0  // PAG 逐帧调试日志（默认关闭，排查问题时可切换 #if 1 临时启用）
  qDebug().noquote().nospace()
      << "[PAG_FRAME] stage=postflush frame=" << frame
      << " changed=" << changed;
#endif

  prepareFramePending_ = false;
  inFlight_.generation = generation_;
  inFlight_.windowEpoch = windowEpoch_;
  inFlight_.serial = ++nextSerial_;
  inFlight_.frame = frame;
  inFlight_.visibilityEpoch = visibilityEpoch_;
  inFlight_.phase = phase;
  ticketInFlight_ = true;
#if 0  // PAG 逐帧调试日志（默认关闭，排查问题时可切换 #if 1 临时启用）
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
#if 0  // 会话级日志（默认关闭；需要跟踪设备就绪重试时可切换 #if 1）
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

// mp4 壁纸锁屏暂停：只停调度与钟，不 cancel、不 unload，保留 player/file/in-flight 语义
// 在途票据仍会自然 swapped，停住画面即锁前最后一帧；暂停点由 nextVisibleFrame_ 记存
void PAGRenderThread::pause() {
  if (!player_ || !file_ || cancelling_) {
    return;
  }
  invalidateFrameSchedule();
  // 作废在途票据：锁屏瞬间的 in-flight 票据可能在屏幕熄灭/合成冻结期间永远等不到
  // swapped 确认（QSG 无 vsync 不渲染），串行票据设计会把整个播放锁死到下次确认。
  // 这里直接清空，迟到的 framePresented 会因票据不匹配被忽略，resume 后可立即续播。
  ticketInFlight_ = false;
#if 0  // 会话级日志（默认关闭；需要跟踪锁屏暂停时可切换 #if 1）
  qDebug().noquote().nospace()
      << "[PAG_SESSION] event=lock-pause gen=" << generation_
      << " atFrame=" << nextVisibleFrame_ << "/" << totalFrames_
      << " discardedInFlight";
#endif
}

// mp4 壁纸解锁恢复：时钟 rebase 到 nextVisibleFrame_ 的帧周期位置
// deadline = framePeriodNs_ * frame - nsecsElapsed，负偏移使恢复后首帧立即提交、
// 后续帧 deadline 全部对齐，避免"冻结到下一帧周期"；rebase 只依赖 nextVisibleFrame_，历史无关
// 若仍有 in-flight 票据（快速锁/解锁交替），只 rebase + 置启动位，由 swap 确认链路自然续链
void PAGRenderThread::resume() {
  if (!player_ || !file_ || cancelling_) {
    return;
  }
  visiblePlaybackStarted_ = true;
  playbackClock_.restart();
  // rebase：把基线拨到"首帧周期槽位"，使首帧在下一个帧周期内出帧、后续帧 deadline 全对齐。
  // 注意不能立即 submitFrame：立即提交会让后续帧的 (elapsed - offset) 补偿错误，
  // 导致解锁后出现 offset(≈暂停帧×帧周期) 级的长停顿（实测 10-17s），wrap 分支已同步清 offset。
  playbackOffsetNs_ = static_cast<qint64>(qMax(0, nextVisibleFrame_ - 1)) * framePeriodNs_;
#if 0  // 会话级日志（默认关闭；需要跟踪解锁恢复时可切换 #if 1）
  qDebug().noquote().nospace()
      << "[PAG_SESSION] event=unlock-resume gen=" << generation_
      << " atFrame=" << nextVisibleFrame_ << "/" << totalFrames_
      << " inFlight=" << ticketInFlight_
      << " offsetNs=" << playbackOffsetNs_;
#endif
  if (ticketInFlight_) {
    return;
  }
  scheduleNextVisibleFrame();
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
#if 0  // 会话级日志（默认关闭；需要跟踪首帧呈现时可切换 #if 1）
    qDebug().noquote().nospace()
        << "[PAG_SESSION] event=prepare-presented gen=" << generation_
        << " serial=" << ticket.serial;
#endif
    Q_EMIT hiddenFramePresented(ticket);
    return;
  }
  if (ticket.frame >= totalFrames_ - 1) {
    if (loopEnabled_) {
      // mp4 壁纸循环：wrap 到帧 0（seek 已前置验证），不 emit playbackFinished
      // 暂停期间（时钟无效）wrap 后 scheduleNextVisibleFrame 自然 no-op，resume 从帧 0 续播
      playbackClock_.restart();
      // 复位 rebase 残留：resume() 曾把 offset 拨到暂停帧周期，若不归零，wrap 后
      // 首帧 deadline 会残留 offset 延迟（= 锁屏暂停帧×帧周期），导致每轮开头卡顿
      playbackOffsetNs_ = 0;
      nextVisibleFrame_ = 0;
#if 0  // 会话级日志（默认关闭；需要跟踪循环回绕时可切换 #if 1）
      qDebug().noquote().nospace()
          << "[PAG_LOOP] event=wrap atFrame=" << ticket.frame << "/" << totalFrames_
          << " serial=" << ticket.serial << " sentToFrame0";
#endif
      scheduleNextVisibleFrame();
      return;
    }
    invalidateFrameSchedule();
#if 0  // 会话级日志（默认关闭；需要跟踪非循环播放结束时切换 #if 1）
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
  const auto remainingNs = deadlineNs - (playbackClock_.nsecsElapsed() - playbackOffsetNs_);
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
  playbackOffsetNs_ = 0;
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
#if 0  // 会话级日志（默认关闭；需要跟踪取消会话时可切换 #if 1）
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
#if 0  // 会话级日志（默认关闭；需要跟踪卸载会话时可切换 #if 1）
    qDebug().noquote().nospace()
        << "[PAG_SESSION] event=safe-unload gen=" << generation
        << " windowEpoch=" << windowEpoch
        << " visibilityEpoch=" << visibilityEpoch;
#endif
    Q_EMIT safeToUnloadWorker(generation, windowEpoch, visibilityEpoch);
  }
}

void PAGRenderThread::unload() {
  const bool hadSession = surface_ || player_ || file_;
  invalidateFrameSchedule();
  prepareFramePending_ = false;
  ticketInFlight_ = false;
  {
    // 释放 GL surface/纹理须与工作线程 flush 串行（见 g_pagGLLock 说明）
    std::lock_guard<std::mutex> glLock(g_pagGLLock);
    // 播放结束/取消时清理 PAG/tgfx 资源缓存（不保证 RSS/PSS 回落）
    if (surface_) {
      surface_->freeCache();
      surface_.reset();
    }
    player_.reset();
    file_.reset();
    totalFrames_ = 0;
  }
  // 仅真实会话卸载时归还 glibc 可回收空闲页，避免空卸载做进程级操作；
  // 须在 g_pagGLLock 外调用（trim 持 arena 锁做 madvise，会阻塞其他 GL 线程）
  if (hadSession) {
    malloc_trim(0);
  }
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
