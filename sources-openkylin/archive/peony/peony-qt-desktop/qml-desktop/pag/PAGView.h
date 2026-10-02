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

#pragma once
#include <QPointer>
#include <QQuickItem>
#include <QScreen>
#include <QMutex>
#include <QTimer>
#include "PAGRenderThread.h"
#include "GPUDrawable.h"

namespace pag {

// 协调仅在解锁时显示的覆盖层与 GUI 线程、Qt Quick 场景图及 PAGRenderThread
// 只有当前帧凭据确认完成交换后，才请求下一帧
class PAGView : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)
  Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)

 public:
  explicit PAGView(QQuickItem* parent = nullptr);
  ~PAGView() override;

  QString filePath() const { return filePath_; }
  bool isActive() const { return active_; }

  Q_INVOKABLE bool prepareUnlock(const QString& filePath, const QString& option,
                                 qulonglong generation, bool staticReady, bool primary,
                                 bool skipStabilityDelay);
  Q_INVOKABLE void beginVisiblePlayback(qulonglong generation);
  Q_INVOKABLE void cancelPlayback();
  Q_INVOKABLE void requestPendingFrameUpdate();
  // mp4 壁纸常驻会话（独立于 unlock 会话）：所有屏各建一个实例
  // locked=true → 仅准备首帧（prepared-paused），待 unlock 后 resumeWallpaperPlayback 起播
  // option 为显示方式（picture-options），空值按 scaled（Zoom）处理
  Q_INVOKABLE bool beginWallpaperPlayback(const QString& filePath, bool locked,
                                          const QString& option = QStringLiteral("scaled"));
  Q_INVOKABLE void stopWallpaperPlayback();
  Q_INVOKABLE void pauseWallpaperPlayback();
  Q_INVOKABLE void resumeWallpaperPlayback();

  void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry) override;
  QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
  void releaseResources() override;
  bool eventFilter(QObject* watched, QEvent* event) override;

 Q_SIGNALS:
  void filePathChanged(const QString& filePath);
  void activeChanged(bool active);
  void prepared(qulonglong generation);
  void finished(qulonglong generation);
  void failed(qulonglong generation, const QString& reason);
  void safeToUnload(qulonglong generation);

 private Q_SLOTS:
  void onWindowChanged(QQuickWindow* window);
  void restartStabilityTimer();
  void prepareAfterStableWindow();
  void onFrameFlushed(const pag::PAGFrameTicket& ticket);
  void onHiddenFramePresented(const pag::PAGFrameTicket& ticket);
  void onPlaybackFinished(const pag::PAGFrameTicket& ticket);
  void onLoadFailed(qulonglong generation, qulonglong windowEpoch,
                    qulonglong visibilityEpoch, const QString& reason);
  void onFrameSwapped();
  void resumeAfterDetach(qulonglong generation, qulonglong windowEpoch,
                         qulonglong visibilityEpoch);
  void onWorkerSafeToUnload(qulonglong generation, qulonglong windowEpoch,
                            qulonglong visibilityEpoch);
  void onCancelAcknowledged(qulonglong generation, qulonglong windowEpoch,
                            qulonglong visibilityEpoch);
  void onSceneGraphInvalidated();

 private:
  bool ticketMatches(const PAGFrameTicket& ticket) const;
  bool windowIsReady() const;
  void bumpWindowEpochAndCancel();
  // mp4 壁纸会话被窗口/屏幕变化取消后的延迟自动重启；锁定（paused_）期间挂起，
  // 解锁 resumeWallpaperPlayback 或暂停态解除后由它接管；stopWallpaperPlayback 清除
  // wallpaperActive_ 后不再重启（防 churn 乒乓）
  void maybeRestartWallpaperAfterCancel();
  void observeScreen(QScreen* screen);
  void notifyNodeDetached();
  int scaleModeForOption(const QString& option) const;

  std::shared_ptr<GPUDrawable> drawable_;
  mutable QMutex stateMutex_{QMutex::Recursive};
  std::unique_ptr<PAGRenderThread> renderThread_;
  QTimer stabilityTimer_;
  QPointer<QQuickWindow> observedWindow_;
  QPointer<QScreen> observedScreen_;
  QString filePath_;
  QString option_;
  // pending 表示工作线程已产出，attached 表示场景图已挂接，swapped 后才能继续下一帧
  PAGFrameTicket pendingTicket_;
  PAGFrameTicket attachedTicket_;
  bool pendingTicketValid_ = false;
  bool attachedTicketValid_ = false;
  bool active_ = false;
  bool cancelling_ = false;
  bool staticReady_ = false;
  bool primary_ = false;
  bool nodeAttached_ = false;
  bool detachPending_ = false;
  bool waitingForDetach_ = false;
  bool loadingStarted_ = false;
  bool skipStabilityDelay_ = false;
  // mp4 壁纸会话状态：wallpaperActive_ 表示常驻会话存在（含被窗口变化取消、等待自动重启）；
  // paused_ 表示锁屏/锁定态（timer 触发重启用它做锁屏复查，见 maybeRestartWallpaperAfterCancel）
  bool wallpaperActive_ = false;
  bool paused_ = false;
  bool wallpaperRestartScheduled_ = false;
  // 解锁 resume 到达时会话仍在取消中（cancelAcknowledged 未回）：挂起恢复意图，
  // 由 onCancelAcknowledged 清 cancelling_ 后转为自动重启；新的 begin/stop 意图覆盖时清除
  bool pendingResumeAfterCancel_ = false;
  int visibleStartRetryCount_ = 0;
  bool visibleFrameRepaintRequested_ = false;
  // 三个代次共同防止旧窗口或旧壁纸的异步回调污染当前会话
  // generation：一次解锁或壁纸请求的会话编号，新的请求会使旧请求全部失效
  // windowEpoch：QQuickWindow/场景图生命周期编号，窗口切换或重建时递增
  // visibilityEpoch：同一窗口内的可见性和壁纸状态编号，取消、重启或主屏变化时递增
  qulonglong generation_ = 0;
  qulonglong windowEpoch_ = 0;
  qulonglong visibilityEpoch_ = 0;
  qulonglong cancelGeneration_ = 0;
  qulonglong cancelWindowEpoch_ = 0;
  qulonglong cancelVisibilityEpoch_ = 0;
};

}  // namespace pag
