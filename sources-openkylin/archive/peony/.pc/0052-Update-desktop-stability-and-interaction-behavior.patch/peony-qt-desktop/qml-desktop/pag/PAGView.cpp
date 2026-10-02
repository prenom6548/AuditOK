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
#include <QSGImageNode>
#include <QQuickWindow>
#include <QScreen>
#include <QGuiApplication>
#include <pag/file.h>
#include <pag/pag.h>
#include "GPUDrawable.h"
#include "PAGRenderThread.h"
#include <ffavc/ffavc.h>

namespace pag {

PAGView::PAGView(QQuickItem* parent) : QQuickItem(parent) {
    // 全局只注册一次 ffavc 软件解码器工厂，供 libpag 解码 H.264 视频图层使用
    static bool decoderRegistered = false;
    if (!decoderRegistered) {
        decoderRegistered = true;
        PAGVideoDecoder::RegisterSoftwareDecoderFactory(
            static_cast<pag::SoftwareDecoderFactory*>(ffavc::DecoderFactory::GetHandle()));
    }

    setFlag(ItemHasContents, true);
    drawable = GPUDrawable::MakeFrom(this);
    pagPlayer = std::make_unique<PAGPlayer>();
    auto pagSurface = PAGSurface::MakeFrom(drawable);
    pagPlayer->setSurface(pagSurface);
    renderThread = std::make_unique<PAGRenderThread>(this);
    renderThread->moveToThread(renderThread.get());
    drawable->moveToThread(renderThread.get());
    renderThread->start();
    // 壁纸属于后台低优先级任务，避免抢占前台应用资源
    renderThread->setPriority(QThread::LowPriority);
    resizeTimer = std::make_unique<QTimer>();
    connect(resizeTimer.get(), &QTimer::timeout, this, &PAGView::sizeChangedDelayHandle);

    playTimer = std::make_unique<QTimer>();
    connect(playTimer.get(), &QTimer::timeout, this, &PAGView::onPlayTimer);
}

PAGView::~PAGView() {
    QMetaObject::invokeMethod(renderThread.get(), "shutDown", Qt::QueuedConnection);
    renderThread->wait();
}

// ---------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------

bool PAGView::isLoop() const {
    return loop_;
}

void PAGView::setLoop(bool loop) {
    if (loop_ == loop) {
        return;
    }
    loop_ = loop;
    Q_EMIT loopChanged(loop);
}

bool PAGView::isPlaying() const {
    return isPlaying_;
}

void PAGView::setIsPlaying(bool isPlaying) {
    if (isPlaying_ == isPlaying) {
        return;
    }
    isPlaying_ = isPlaying;
    Q_EMIT isPlayingChanged(isPlaying);
    if (isPlaying) {
        if (pagFile) {
            auto fps = pagFile->frameRate();
            if (fps <= 0.0) fps = 15.0;
            // 壁纸场景 15fps 视觉无差异，钳位以大幅降低 CPU 占用
            if (fps > 15.0) fps = 15.0;
            playTimer->start(static_cast<int>(1000.0 / fps));
        }
        QMetaObject::invokeMethod(renderThread.get(), "flush", Qt::QueuedConnection);
    } else {
        playTimer->stop();
    }
}

double PAGView::getProgress() const {
    return progress;
}

void PAGView::setProgress(double progress) {
    if (this->progress == progress) {
        return;
    }
    setProgressInternal(progress);
}

QString PAGView::getFilePath() const {
    if (pagFile == nullptr) {
        return QString();
    }
    return QString::fromLocal8Bit(pagFile->path().data());
}

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void PAGView::flush() const {
    if (isPlaying_) {
        QMetaObject::invokeMethod(renderThread.get(), "flush", Qt::QueuedConnection);
    }
}

void PAGView::sizeChangedDelayHandle() {
    resizeTimer->stop();
    // 只在尺寸真正发生变化时才触发 updateSize()，避免初始化后的冗余重建
    auto dpr = window() ? window()->devicePixelRatio() : 1.0;
    int newW = static_cast<int>(width()  * dpr);
    int newH = static_cast<int>(height() * dpr);
    if (drawable && newW == drawable->width() && newH == drawable->height()) {
        return;
    }
    sizeChanged = true;
    QMetaObject::invokeMethod(renderThread.get(), "flush", Qt::QueuedConnection);
}

// ---------------------------------------------------------------------------
// Qt overrides
// ---------------------------------------------------------------------------

void PAGView::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry) {
    if (newGeometry == oldGeometry) {
        return;
    }
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    resizeTimer->start(400);
}

bool PAGView::setFile(const QString& filePath) {
    qWarning("PAGView::setFile called with: %s", filePath.toLocal8Bit().data());
    if (filePath.isEmpty()) {
        return false;
    }
    auto strPath = std::string(filePath.toLocal8Bit());
    if (filePath.startsWith(QLatin1String("file://"))) {
        strPath = std::string(QUrl(filePath).toLocalFile().toLocal8Bit());
    }
    auto byteData = pag::ByteData::FromPath(strPath);
    if (byteData == nullptr) {
        qWarning("PAGView::setFile: failed to read file: %s", strPath.c_str());
        return false;
    }
    auto newPagFile = pag::PAGFile::Load(byteData->data(), byteData->length());
    if (newPagFile == nullptr) {
        qWarning("PAGView::setFile: failed to load PAG file: %s", strPath.c_str());
        return false;
    }

    setIsPlaying(false);
    firstFrameEmitted_ = false;
    firstFrameRendered_ = false;
    pagFile = newPagFile;
    pagFile->getFile()->path = strPath;
    pagPlayer->setComposition(pagFile);
    pagPlayer->setScaleMode(scaleMode_);

    sizeChanged = true;
    // 取消由初始化 geometry 触发的待定 timer，避免它在首帧渲染后再次调用 updateSize()
    // 导致 GPU 纹理短暂被清空，造成一帧黑色闪烁。
    resizeTimer->stop();
    playbackFinished_ = false;

    // progressPerFrame is used for manual frame stepping (not for wallpaper auto-play,
    // but kept for API completeness).
    auto frameRate = pagFile->frameRate();
    auto durationUs = pagFile->duration();  // microseconds
    if (frameRate > 0.0 && durationUs > 0) {
        progressPerFrame = 1.0 / (frameRate * static_cast<double>(durationUs) / 1000000.0);
    } else {
        progressPerFrame = 0.0;
    }

    Q_EMIT filePathChanged(QString::fromLocal8Bit(strPath.c_str()));
    setProgressInternal(0.0);
    setIsPlaying(true);
    return true;
}

// ---------------------------------------------------------------------------
// Scene graph / render
// ---------------------------------------------------------------------------

QSGNode* PAGView::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) {
    auto node = static_cast<QSGImageNode*>(oldNode);
    auto texture = drawable->getTexture();
    if (texture) {
        if (node == nullptr) {
            node = window()->createImageNode();
        }
        node->setTexture(texture);
        node->markDirty(QSGNode::DirtyMaterial);
        // Fill the entire item bounding rect (stretch mode).
        // The QML item is expected to be anchors.fill: parent so this
        // effectively fills the whole desktop viewport.
        node->setRect(boundingRect());

        // 必须同时满足：
        //   1. firstFrameRendered_：渲染线程已确认 pagPlayer->flush() 画出了第一帧
        //      （防止 getTexture() 返回初始化时创建的空纹理就触发信号）
        //   2. !firstFrameEmitted_：信号只发一次
        if (firstFrameRendered_ && !firstFrameEmitted_) {
            firstFrameEmitted_ = true;
            QMetaObject::invokeMethod(this, [this]() {
                Q_EMIT firstFrameReady();
            }, Qt::QueuedConnection);
        }
    }

    return node;
}

void PAGView::onPlayTimer() {
    if (!isPlaying_ || playbackFinished_) {
        return;
    }

    auto newProgress = this->progress + progressPerFrame;
    if (newProgress >= 1.0) {
        if (loop_) {
            // 循环：回到起点
            setProgressInternal(0.0);
        } else {
            // 单次播放：停在最后一帧并通知
            playbackFinished_ = true;
            playTimer->stop();
            setProgressInternal(1.0);
            QMetaObject::invokeMethod(this, [this]() {
                setIsPlaying(false);
                Q_EMIT finished();
            }, Qt::QueuedConnection);
        }
    } else {
        setProgressInternal(newProgress);
    }
}

PAGRenderThread* PAGView::getRenderThread() const {
    return renderThread.get();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void PAGView::setProgressInternal(double progress) {
    pagPlayer->setProgress(progress);
    this->progress = progress;
    Q_EMIT progressChanged(progress);
    QMetaObject::invokeMethod(renderThread.get(), "flush", Qt::QueuedConnection);
}

void PAGView::restart() {
    if (!pagFile) {
        return;
    }
    playbackFinished_ = false;
    firstFrameEmitted_ = false;
    firstFrameRendered_ = false;
    setProgressInternal(0.0);
    setIsPlaying(true);
}

void PAGView::setScaleModeFromOption(const QString& option) {
    PAGScaleMode mode = PAGScaleMode::Stretch;
    auto opt = option.toLower();
    if (opt == QLatin1String("scaled")) {
        mode = PAGScaleMode::Zoom;       // PreserveAspectCrop：保持比例，超出裁剪
    } else if (opt == QLatin1String("zoom")) {
        mode = PAGScaleMode::LetterBox;  // PreserveAspectFit：保持比例，两边留空
    }
    // "stretched" 及其他模式均使用 Stretch
    scaleMode_ = mode;
    if (pagPlayer) {
        pagPlayer->setScaleMode(scaleMode_);
        QMetaObject::invokeMethod(renderThread.get(), "flush", Qt::QueuedConnection);
    }
}

}  // namespace pag
