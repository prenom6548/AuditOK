/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making tgfx available.
//
//  Copyright (C) 2023 Tencent. All rights reserved.
//
//  Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
//  in compliance with the License. You may obtain a copy of the License at
//
//      https://opensource.org/licenses/BSD-3-Clause
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "tgfx/gpu/opengl/qt/QGLWindow.h"
#include <QApplication>
#include <QDebug>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QThread>
#include "gpu/opengl/GLTexture.h"

namespace tgfx {
// QSGTexture wrapper 生命周期管理：
// 每帧通过 QQuickWindow::createTextureFromNativeObject 新建 QSGPlainTexture 并交给
// QSGImageNode，旧 wrapper 进入 QGLDeviceCreator 的 4 槽延迟回收 ring（详见
// retireTexture），避免"wrapper 在 QSGBatchRenderer 仍引用时被释放"的 UAF，同时
// 保证 Qt 材质层能通过 textureId 变化触发 batch 刷新（自定义 QSGTexture 子类因
// comparisonKey 非虚+默认私有类恒等而无法刷新，会导致黑屏）。
class QGLDeviceCreator : public QObject {
  Q_OBJECT
 public:
  QGLDeviceCreator(QQuickItem* item, QGLWindow* window) : item(item), window(window) {
    connect(item, &QObject::destroyed, this, [this]() {
      QMutexLocker lock(&this->window->quickItemMutex);
      this->item = nullptr;
    }, Qt::DirectConnection);
    auto nativeWindow = item->window();
    if (nativeWindow != nullptr) {
      handleWindowChanged(nativeWindow);
    } else {
      connect(item, SIGNAL(windowChanged(QQuickWindow*)), this,
              SLOT(handleWindowChanged(QQuickWindow*)));
    }
  }

  ~QGLDeviceCreator() override {
    QMutexLocker lock(&window->quickItemMutex);
    if (item) {
      disconnect(item, SIGNAL(windowChanged(QQuickWindow*)), this,
                 SLOT(handleWindowChanged(QQuickWindow*)));
      auto nativeWindow = item->window();
      if (nativeWindow != nullptr) {
        disconnect(nativeWindow, SIGNAL(beforeRendering()), this, SLOT(onBeforeRendering()));
        disconnect(nativeWindow, SIGNAL(sceneGraphInvalidated()), this,
                   SLOT(onSceneGraphInvalidated()));
      }
    }
    // 兜底：释放 ring 中仍存活的所有 wrapper（QGLWindow 析构时渲染线程已停止）。
    releaseAllRetired();
  }

  // 仅在 QSG 渲染线程调用（getQSGTexture 内）。
  // 将旧 QSGTexture wrapper 收入 4 槽延迟回收 ring：每入一个新对象便淘汰并释放
  // 最老的一个，保证 wrapper 在被释放前至少历经 4 个渲染周期，覆盖
  // QSGBatchRenderer 跨帧材质状态对上一帧纹理的引用（afterRendering 时机过早，
  // 不能在那里释放）。
  void retireTexture(QSGTexture* texture) {
    QSGTexture* evicted = retiredSlots_[retireHead_];
    if (evicted != nullptr) {
      delete evicted;
    }
    retiredSlots_[retireHead_] = texture;
    retireHead_ = (retireHead_ + 1) % kRetiredSlotCount;
  }

 private:
  static constexpr int kRetiredSlotCount = 4;
  QQuickItem* item = nullptr;
  QGLWindow* window = nullptr;
  // 延迟回收 ring（仅 QSG 渲染线程访问）：旧 wrapper 保留至少 4 个渲染周期，
  // 覆盖 QSGBatchRenderer 跨帧材质状态对上一帧纹理的引用。
  QSGTexture* retiredSlots_[kRetiredSlotCount] = {nullptr, nullptr, nullptr, nullptr};
  int retireHead_ = 0;

  void releaseAllRetired() {
    for (int i = 0; i < kRetiredSlotCount; ++i) {
      if (retiredSlots_[i] != nullptr) {
        delete retiredSlots_[i];
        retiredSlots_[i] = nullptr;
      }
    }
    retireHead_ = 0;
  }

  Q_SLOT
  void handleWindowChanged(QQuickWindow* nativeWindow) {
    disconnect(item, SIGNAL(windowChanged(QQuickWindow*)), this,
               SLOT(handleWindowChanged(QQuickWindow*)));
    if (nativeWindow != nullptr) {
      auto shareContext = getShareContext(nativeWindow);
      if (shareContext && shareContext->thread() == QThread::currentThread()) {
        // We are on the same thread as the QSG render thread, so we can create the device here.
        createDevice(shareContext);
      } else {
        connect(nativeWindow, SIGNAL(beforeRendering()), this, SLOT(onBeforeRendering()),
                Qt::DirectConnection);
        connect(nativeWindow, SIGNAL(sceneGraphInvalidated()), this,
                SLOT(onSceneGraphInvalidated()), Qt::DirectConnection);
        QMetaObject::invokeMethod(item, "update", Qt::AutoConnection);
      }
    }
  }

  Q_SLOT
  void onBeforeRendering() {
    // onBeforeRendering runs on the QSG render thread; item is written on the GUI thread.
    // Snapshot nativeWindow under quickItemMutex to avoid a TOCTOU race on item.
    QQuickWindow* nativeWindow = nullptr;
    {
      QMutexLocker lock(&window->quickItemMutex);
      if (!item) {
        return;
      }
      nativeWindow = item->window();
    }
    if (!nativeWindow) {
      return;
    }
    disconnect(nativeWindow, SIGNAL(beforeRendering()), this, SLOT(onBeforeRendering()));
    connect(nativeWindow, SIGNAL(sceneGraphInvalidated()), this,
            SLOT(onSceneGraphInvalidated()), Qt::DirectConnection);
    auto shareContext = getShareContext(nativeWindow);
    createDevice(shareContext);
  }

  Q_SLOT
  void onSceneGraphInvalidated() {
    // 场景图失效（窗口销毁/上下文重建）后所有 wrapper 均不再被任何 batch 引用，
    // 此时连同当前 outTexture 一并释放，防止重建后残留悬空包装。
    QMutexLocker lock(&window->quickItemMutex);
    QSGTexture* out = window->outTexture;
    if (out != nullptr) {
#if 0  // 诊断日志：event=texture-scenegraph-invalidated
      qDebug().noquote().nospace()
          << "[PAG_GL] event=texture-scenegraph-invalidated this=" << window
          << " outTexture=" << out
          << " thread=" << QThread::currentThread();
#endif
      delete out;
      window->outTexture = nullptr;
    }
    releaseAllRetired();
  }

  QOpenGLContext* getShareContext(QQuickWindow* nativeWindow) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return reinterpret_cast<QOpenGLContext*>(nativeWindow->rendererInterface()->getResource(
        nativeWindow, QSGRendererInterface::OpenGLContextResource));
#else
    return nativeWindow->openglContext();
#endif
  }

  void createDevice(QOpenGLContext* shareContext) {
    // Creating a context that shares with a context that is current on another thread is not safe,
    // some drivers on windows can reject this. So we have to create the context here on the QSG
    // render thread.
    auto context = new QOpenGLContext();
    context->setFormat(shareContext->format());
    context->setShareContext(shareContext);
    context->create();
    auto app = QApplication::instance();
    context->moveToThread(app->thread());
    QMetaObject::invokeMethod(
        app,
        [weak = window->weakThis, context] {
          auto self = weak.lock();
          if (!self) {
            delete context;
            return;
          }
          self->createDevice(context);
        },
        Qt::AutoConnection);
  }
};

std::shared_ptr<QGLWindow> QGLWindow::MakeFrom(QQuickItem* quickItem, bool singleBufferMode) {
  if (quickItem == nullptr) {
    return nullptr;
  }
  auto window = std::shared_ptr<QGLWindow>(new QGLWindow(quickItem, singleBufferMode));
  window->weakThis = window;
  window->initDevice();
  return window;
}

QGLWindow::QGLWindow(QQuickItem* quickItem, bool singleBufferMode)
    : quickItem(quickItem), singleBufferMode(singleBufferMode) {
#if 0  // 诊断日志：event=qglwindow-construct
  qDebug().noquote().nospace()
      << "[PAG_GL] event=qglwindow-construct this=" << this
      << " quickItem=" << quickItem
      << " singleBufferMode=" << singleBufferMode
      << " thread=" << QThread::currentThread();
#endif
  if (QThread::currentThread() != QApplication::instance()->thread()) {
    renderThread = QThread::currentThread();
  }
}

QGLWindow::~QGLWindow() {
#if 0  // 诊断日志：event=qglwindow-destroy
  qDebug().noquote().nospace()
      << "[PAG_GL] event=qglwindow-destroy this=" << this
      << " thread=" << QThread::currentThread()
      << " outTexture=" << outTexture
      << " pendingSurface=" << pendingSurface.get()
      << " displayingSurface=" << displayingSurface.get();
#endif
  // outTexture 是当前帧 wrapper，所有权移交给 deviceCreator 的延迟回收 ring，
  // 由 deviceCreator 析构统一释放（ring 全部对象）。这里不再自行 delete，
  // 防止与 ring 的 eviction/releaseAll 双重释放。
  if (outTexture != nullptr) {
    deviceCreator->retireTexture(outTexture);
    outTexture = nullptr;
  }
  delete deviceCreator;
}

void QGLWindow::moveToThread(QThread* thread) {
#if 0  // 诊断日志：event=qglwindow-move
  qDebug().noquote().nospace() << "[PAG_GL] event=qglwindow-move this=" << this
                               << " from=" << QThread::currentThread()
                               << " to=" << thread;
#endif
  std::lock_guard<std::mutex> autoLock(locker);
  renderThread = thread;
  if (device != nullptr) {
    static_cast<QGLDevice*>(device.get())->moveToThread(renderThread);
  }
}

QSGTexture* QGLWindow::getQSGTexture() {
  std::lock_guard<std::mutex> autoLock(locker);
  QMutexLocker quickItemLocker(&quickItemMutex);
  if (quickItem == nullptr || device == nullptr) {
    qDebug().noquote().nospace() << "[PAG_GL] event=get-texture-skip this=" << this
                                 << " reason=" << (quickItem == nullptr ? "no-item" : "no-device");
    return nullptr;
  }
  auto nativeWindow = quickItem->window();
  if (nativeWindow == nullptr) {
    qDebug().noquote().nospace() << "[PAG_GL] event=get-texture-skip this=" << this
                                 << " reason=no-window";
    return nullptr;
  }
  if (pendingTextureID > 0 && pendingSurface != nullptr) {
    auto width = static_cast<int>(ceil(quickItem->width()));
    auto height = static_cast<int>(ceil(quickItem->height()));
    // 每帧新建 QSGPlainTexture（compare()/comparisonKey 依赖 textureId 变化触发
    // batch 刷新；复用同一 wrapper 对象会导致 Qt 材质层判定纹理未变而黑屏）。
    // 旧 wrapper 不立即释放，移交 ring 延迟回收（afterRendering 提前释放会 UAF）。
    if (outTexture != nullptr) {
      deviceCreator->retireTexture(outTexture);
      outTexture = nullptr;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    outTexture = QNativeInterface::QSGOpenGLTexture::fromNative(
        pendingTextureID, nativeWindow, QSize(width, height), QQuickWindow::TextureHasAlphaChannel);

#elif (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    outTexture = nativeWindow->createTextureFromNativeObject(
        QQuickWindow::NativeObjectTexture, &pendingTextureID, 0, QSize(width, height),
        QQuickWindow::TextureHasAlphaChannel);
#else
    outTexture = nativeWindow->createTextureFromId(pendingTextureID, QSize(width, height),
                                                   QQuickWindow::TextureHasAlphaChannel);
#endif
    if (outTexture == nullptr) {
      qDebug().noquote().nospace()
          << "[PAG_GL] event=texture-create-failed this=" << this
          << " nativeTextureID=" << pendingTextureID
          << " pendingSurface=" << pendingSurface.get();
    }
    // Keep the surface alive until the next getQSGTexture() call.
    displayingSurface = pendingSurface;
    pendingSurface = nullptr;
    pendingTextureID = 0;
    if (!singleBufferMode) {
      std::swap(fontSurface, surface);
    }
  }
  return outTexture;
}

std::shared_ptr<Surface> QGLWindow::onCreateSurface(Context* context) {
  QMutexLocker quickItemLocker(&quickItemMutex);
  if (quickItem == nullptr) {
    return nullptr;
  }
  auto nativeWindow = quickItem->window();
  if (nativeWindow == nullptr) {
    return nullptr;
  }
  auto pixelRatio = nativeWindow->devicePixelRatio();
  auto width = static_cast<int>(ceil(quickItem->width() * pixelRatio));
  auto height = static_cast<int>(ceil(quickItem->height() * pixelRatio));
  if (width <= 0 || height <= 0) {
    return nullptr;
  }
  if (!singleBufferMode) {
    fontSurface = Surface::Make(context, width, height, ColorType::RGBA_8888);
    if (fontSurface == nullptr) {
      return nullptr;
    }
  }
  auto backSurface = Surface::Make(context, width, height, ColorType::RGBA_8888);
  if (backSurface == nullptr) {
    fontSurface = nullptr;
    return nullptr;
  }
  sizeInvalid = false;
  return backSurface;
}

void QGLWindow::onPresent(Context*, int64_t) {
  GLTextureInfo textureInfo = {};
  // Surface->getBackendTexture() triggers a flush, so we need to call it under the context.
  surface->getBackendTexture().getGLTextureInfo(&textureInfo);
  pendingTextureID = textureInfo.id;
  // Keep the surface alive until the next getQSGTexture() call.
  pendingSurface = surface;
  // Hold quickItemMutex during invokeMethod so that destroyed() DirectConnection
  // cannot release quickItem until after the post is enqueued.
  QMutexLocker quickItemLocker(&quickItemMutex);
  if (quickItem) {
    QMetaObject::invokeMethod(quickItem, "update", Qt::QueuedConnection);
  }
}

void QGLWindow::onFreeSurface() {
#if 0  // 诊断日志：event=free-surface
  qDebug().noquote().nospace() << "[PAG_GL] event=free-surface this=" << this
                               << " thread=" << QThread::currentThread()
                               << " fontSurface=" << fontSurface.get()
                               << " surface=" << surface.get()
                               << " pendingSurface=" << pendingSurface.get()
                               << " displayingSurface=" << displayingSurface.get();
#endif
  fontSurface = nullptr;
  surface = nullptr;
}

void QGLWindow::initDevice() {
  deviceCreator = new QGLDeviceCreator(quickItem, this);
  QObject::connect(quickItem, &QObject::destroyed, deviceCreator, [this]() {
    QMutexLocker autoLock(&quickItemMutex);
    quickItem = nullptr;
  }, Qt::DirectConnection);
}

void QGLWindow::createDevice(QOpenGLContext* context) {
  {
    std::lock_guard<std::mutex> autoLock(locker);
    auto surface = new QOffscreenSurface();
    surface->setFormat(context->format());
    surface->create();
    device = QGLDevice::MakeFrom(context, surface, true);
    if (renderThread != nullptr) {
      static_cast<QGLDevice*>(device.get())->moveToThread(renderThread);
    }
  }
#if 0  // 诊断日志：event=device-created
  qDebug().noquote().nospace() << "[PAG_GL] event=device-created this=" << this
                               << " thread=" << QThread::currentThread()
                               << " device=" << device.get()
                               << " context=" << context
                               << " renderThread=" << renderThread;
#endif
  QQuickItem* localItem = nullptr;
  {
    QMutexLocker quickItemLocker(&quickItemMutex);
    localItem = quickItem;
  }
  if (localItem) {
    QMetaObject::invokeMethod(localItem, "update", Qt::AutoConnection);
  }
}
}  // namespace tgfx

#include "QGLWindow.moc"
