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

#pragma once

#include <QElapsedTimer>
#include <QThread>
#include <QTimer>
#include <QString>
#include <memory>
#include <pag/pag.h>

namespace pag {

class GPUDrawable;

// 标识一帧在工作线程生成、场景图挂接和交换确认三个阶段的状态
struct PAGFrameTicket {
  enum Phase {
    PrepareHidden = 0,
    VisiblePlayback = 1
  };

  // generation 隔离不同解锁会话，windowEpoch/visibilityEpoch 隔离窗口和显示状态变化
  // generation 区分不同解锁会话
  // windowEpoch 区分不同窗口或场景图
  // visibilityEpoch 区分同一窗口内不同的可见播放状态
  qulonglong generation = 0;
  qulonglong windowEpoch = 0;
  qulonglong serial = 0;
  int frame = 0;
  qulonglong visibilityEpoch = 0;
  int phase = PrepareHidden;

  bool operator==(const PAGFrameTicket& other) const {
    return generation == other.generation && windowEpoch == other.windowEpoch &&
           serial == other.serial && frame == other.frame &&
           visibilityEpoch == other.visibilityEpoch && phase == other.phase;
  }
};

// 持有 PAG 解码和渲染对象，确保加载、刷新和卸载操作均在专用工作线程中执行
class PAGRenderThread : public QThread {
  Q_OBJECT
 public:
  explicit PAGRenderThread(const std::shared_ptr<GPUDrawable>& drawable);

 Q_SIGNALS:
  void loadFailed(qulonglong generation, qulonglong windowEpoch,
                  qulonglong visibilityEpoch, const QString& reason);
  void frameFlushed(const pag::PAGFrameTicket& ticket);
  void hiddenFramePresented(const pag::PAGFrameTicket& ticket);
  void playbackFinished(const pag::PAGFrameTicket& ticket);
  void cancelAcknowledged(qulonglong generation, qulonglong windowEpoch,
                          qulonglong visibilityEpoch);
  void safeToUnloadWorker(qulonglong generation, qulonglong windowEpoch,
                          qulonglong visibilityEpoch);

 public Q_SLOTS:
  void loadAndPrepare(const QString& filePath, int scaleMode, int pixelWidth, int pixelHeight,
                      qulonglong generation, qulonglong windowEpoch,
                      qulonglong visibilityEpoch);
  // mp4 壁纸常驻会话：循环播放（末帧 wrap，不 emit playbackFinished）
  void loadVideoLoop(const QString& filePath, int scaleMode, int pixelWidth, int pixelHeight,
                     qulonglong generation, qulonglong windowEpoch,
                     qulonglong visibilityEpoch);
  // mp4 壁纸锁屏暂停/解锁恢复：暂停保留全部资源，恢复按 nextVisibleFrame_ 重定时钟基线
  void pause();
  void resume();
  void startVisiblePlayback(qulonglong generation, qulonglong windowEpoch,
                            qulonglong visibilityEpoch);
  void retryPendingPrepare(qulonglong generation, qulonglong windowEpoch,
                           qulonglong visibilityEpoch);
  void framePresented(const pag::PAGFrameTicket& ticket);
  void cancel(qulonglong generation, qulonglong windowEpoch, qulonglong visibilityEpoch);
  void cancelEmptySession(qulonglong generation, qulonglong windowEpoch,
                          qulonglong visibilityEpoch);
  void nodeDetached(qulonglong generation, qulonglong windowEpoch, qulonglong visibilityEpoch);
  void shutDown();

 private:
  bool matches(qulonglong generation, qulonglong windowEpoch,
               qulonglong visibilityEpoch) const;
  void submitFrame(int frame, PAGFrameTicket::Phase phase);
  void scheduleNextVisibleFrame();
  void invalidateFrameSchedule();
  void unloadIfDetached();
  void unload();
  // loadAndPrepare 与 loadVideoLoop 的公共加载体（循环标志由调用方先行设置）
  void loadCommon(const QString& filePath, int scaleMode, int pixelWidth, int pixelHeight,
                  qulonglong generation, qulonglong windowEpoch,
                  qulonglong visibilityEpoch);

  std::shared_ptr<GPUDrawable> drawable_;
  std::unique_ptr<PAGPlayer> player_;
  std::shared_ptr<PAGFile> file_;
  PAGFrameTicket inFlight_;
  QElapsedTimer playbackClock_;
  // resume 时钟 rebase 偏移：playbackClock_.nsecsElapsed() - playbackOffsetNs_ 为帧时钟
  // （QElapsedTimer 无加偏移 API，仅 resume 设置；unlock 路径恒为 0，行为不变）
  qint64 playbackOffsetNs_ = 0;
  bool ticketInFlight_ = false;
  bool visiblePlaybackStarted_ = false;
  bool cancelling_ = false;
  bool nodeDetached_ = true;
  bool safeUnloadEmitted_ = false;
  bool prepareFramePending_ = false;
  qulonglong generation_ = 0;
  qulonglong windowEpoch_ = 0;
  qulonglong visibilityEpoch_ = 0;
  qulonglong nextSerial_ = 0;
  qulonglong scheduleEpoch_ = 0;
  int totalFrames_ = 0;
  int nextVisibleFrame_ = 0;
  qint64 framePeriodNs_ = 33333333;
  // mp4 壁纸循环模式：末帧 presented 后 wrap 到帧 0（不 emit playbackFinished）
  bool loopEnabled_ = false;
};

}  // namespace pag

Q_DECLARE_METATYPE(pag::PAGFrameTicket)
