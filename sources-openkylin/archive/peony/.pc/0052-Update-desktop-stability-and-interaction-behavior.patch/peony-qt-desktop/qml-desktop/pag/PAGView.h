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

#include <QOpenGLContext>
#include <QQuickItem>
#include <QTimer>
#include <pag/pag.h>
#include "PAGRenderThread.h"
#include "GPUDrawable.h"

namespace pag {

/**
 * PAGView — Qt Quick item that renders a .pag animation file onto the desktop wallpaper.
 *
 * Key modifications compared to upstream UKUIQuickPAGViewer:
 *  - Added `loop` property (default false). When false the animation stops at the last frame
 *    and emits the `finished` signal instead of wrapping around to 0.
 *  - Removed audio-related code (unused for wallpaper rendering).
 *  - Removed frameTimeMetricsReady signal (not needed here).
 *  - `setFile()` no longer forces `setSize()` to the PAG preferred size; the item is expected
 *    to be sized by the parent (anchors.fill: parent) so it always fills the viewport.
 */
class PAGView : public QQuickItem {
  Q_OBJECT
 public:
  explicit PAGView(QQuickItem* parent = nullptr);
  ~PAGView() override;

  // ---- QML-exposed properties ----

  /** Whether the animation loops forever. Default: false (play-once). */
  Q_PROPERTY(bool loop READ isLoop WRITE setLoop NOTIFY loopChanged)

  Q_PROPERTY(bool isPlaying READ isPlaying WRITE setIsPlaying NOTIFY isPlayingChanged)
  Q_PROPERTY(double progress READ getProgress WRITE setProgress NOTIFY progressChanged)
  Q_PROPERTY(QString filePath READ getFilePath NOTIFY filePathChanged)

  // ---- Accessors ----

  bool isLoop() const;
  bool isPlaying() const;
  double getProgress() const;
  QString getFilePath() const;

  // ---- Mutators ----

  void setLoop(bool loop);
  void setIsPlaying(bool isPlaying);
  void setProgress(double progress);

  // ---- Qt overrides ----

  void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry) override;
  QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*) override;

  // ---- Invokable methods ----

  /** Load and start playing the given .pag file path. Returns false on failure. */
  Q_INVOKABLE bool setFile(const QString& filePath);

  // ---- Signals ----

  Q_SIGNAL void loopChanged(bool loop);
  Q_SIGNAL void isPlayingChanged(bool isPlaying);
  Q_SIGNAL void progressChanged(double progress);
  Q_SIGNAL void filePathChanged(const QString& filePath);
  /** Emitted when a non-looping animation reaches the last frame and stops. */
  Q_SIGNAL void finished();
  /** Emitted once the render thread has successfully flushed the very first frame.
   *  QML can use this to hide any placeholder / show desktop icons. */
  Q_SIGNAL void firstFrameReady();

  // ---- Slots ----

  Q_SLOT void flush() const;
  Q_SLOT void sizeChangedDelayHandle();
  Q_SLOT void onPlayTimer();

  // ---- Invokable control methods ----

  /** Restart playback from the beginning without reloading the file.
   *  Resets progress/finished state and re-emits firstFrameReady on the next render. */
  Q_INVOKABLE void restart();

  /** Map a BackgroundItem backgroundOption string to the corresponding PAGScaleMode and apply it.
   *  Supported: "stretched" → Stretch, "scaled" → Zoom, "zoom" → LetterBox.
   *  Any other value falls back to Stretch. */
  Q_INVOKABLE void setScaleModeFromOption(const QString& option);

  PAGRenderThread* getRenderThread() const;

 private:
  void setProgressInternal(double progress);

  bool loop_ = false;
  bool isPlaying_ = true;
  bool playbackFinished_ = false;   ///< true once a non-loop animation has ended
  PAGScaleMode scaleMode_ = PAGScaleMode::Stretch;  ///< current scale mode, applied on each setFile()
  bool firstFrameEmitted_ = false;   ///< true once firstFrameReady has been emitted for the current file
  std::atomic_bool firstFrameRendered_ = false;  ///< render-thread flag: first frame actually drawn by pagPlayer
  std::atomic_bool sizeChanged = false;
  double progress = 0.0;
  double progressPerFrame = 0.0;
  std::unique_ptr<QTimer> resizeTimer = nullptr;
  std::unique_ptr<QTimer> playTimer = nullptr;
  std::unique_ptr<PAGPlayer> pagPlayer = nullptr;
  std::unique_ptr<PAGRenderThread> renderThread = nullptr;
  std::shared_ptr<PAGFile> pagFile = nullptr;
  std::shared_ptr<GPUDrawable> drawable = nullptr;

  friend class PAGRenderThread;
};

}  // namespace pag
