/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: sundagao <sundagao@kylinos.cn>
 *
 */

#include <algorithm>
#include <QSGImageNode>
#include <QQuickWindow>
#include <QThread>
#include <QOpenGLContext>
#include "thumbnail-item.h"
#include "discard-texture-resources-runnable.h"
using namespace UkuiThumbnail;


QSGTexture* UkuiThumbnail::ThumbnailTextureProvider::texture() const
{
    return m_texture;
}

void UkuiThumbnail::ThumbnailTextureProvider::updateTexture(QSGTexture* texture)
{
    m_texture = texture;
    Q_EMIT textureChanged();
}

ThumbnailItem::ThumbnailItem(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    connect(this, &QQuickItem::visibleChanged, this, [this]() {
        active(shouldBeActive());
    });
    connect(this, &QQuickItem::windowChanged, this, &ThumbnailItem::onWindowChanged);
}

ThumbnailItem::~ThumbnailItem()
{
    if (m_releaseQueue && m_thumbnail) {
        m_releaseQueue->enqueue(m_thumbnail->takeAllBuffers());
    }

    if (m_window) {
        disconnect(m_window, nullptr, this, nullptr);
    }
    if (m_textureProvider) {
        m_textureProvider->updateTexture(nullptr);
        delete m_textureProvider;
        m_textureProvider = nullptr;
    }
    if (m_thumbnail) {
        delete m_thumbnail;
        m_thumbnail = nullptr;
    }
}

QString ThumbnailItem::uuid() const
{
    return m_uuid;
}

void ThumbnailItem::setUuid(const QString& uuid)
{
    if(m_uuid == uuid) {
        return;
    }
    m_uuid = uuid;
    thumbnail()->setUuid(uuid);
    active(shouldBeActive());
    Q_EMIT uuidChanged();
}

bool ThumbnailItem::fixHeight() const
{
    return m_fixHeight;;
}

void ThumbnailItem::setFixHeight(bool fixHeight)
{
    if(m_fixHeight == fixHeight) {
        return;
    }
    m_fixHeight = fixHeight;
    Q_EMIT fixHeightChanged();
}

bool ThumbnailItem::fixWidth() const
{
    return m_fixWidth;
}

void ThumbnailItem::setFixWidth(bool fixWidth)
{
    if(m_fixWidth == fixWidth) {
        return;
    }
    m_fixWidth = fixWidth;
    Q_EMIT fixWidthChanged();
}

void ThumbnailItem::onWindowChanged(QQuickWindow* window)
{
    if (m_window == window && window) {
        return;
    }
    if (m_window) {
        disconnect(m_window, nullptr, this, nullptr);
    }
    m_window = window;
    if (m_window) {
        m_releaseQueue = SceneGraphResourceReleaseQueue::fromWindow(window);
        connect(m_window, &QQuickWindow::afterRendering, this, &ThumbnailItem::onAfterRendering, Qt::DirectConnection);
        connect(m_window, &QQuickWindow::visibleChanged, this, [this]() {
            const bool activeNow = shouldBeActive();
            active(activeNow);
            if (activeNow) {
                update();
            }
        });
        connect(m_window, &QQuickWindow::sceneGraphAboutToStop, this, [this]() {
            m_nodeNeedsRebuild.store(true, std::memory_order_release);
            QMetaObject::invokeMethod(this, [this]() {
                update();
            }, Qt::QueuedConnection);
                if (m_releaseQueue && m_thumbnail) {
                    m_releaseQueue->enqueue(m_thumbnail->takeAllBuffers(), false);
                }
        }, Qt::DirectConnection);
    }
    active(shouldBeActive());
}

void ThumbnailItem::active(bool active)
{
    auto* currentThumbnail = thumbnail();
    if (active) {
        if (!currentThumbnail->start()) {
            qWarning() <<"Thumbnail failed to startisSceneGraphInitialized: " << m_uuid;
        }
    } else {
        currentThumbnail->stop();
    }
}

QSGNode* ThumbnailItem::updatePaintNode(QSGNode* oldNode, QQuickItem::UpdatePaintNodeData* )
{
    if (m_nodeNeedsRebuild.exchange(false, std::memory_order_acq_rel)) {
        delete oldNode;
        oldNode = nullptr;
    }

    auto* texture = textureProvider()->texture();
    if (!texture) {
        delete oldNode;
        m_renderPending = false;
        return nullptr;
    }
    auto* textureNode = dynamic_cast<QSGImageNode *>(oldNode);
    if (!textureNode) {
        textureNode = window()->createImageNode();
        textureNode->setOwnsTexture(false);
        textureNode->setFiltering(QSGTexture::Linear);
    }
    textureNode->setTexture(texture);
    QSizeF scaleSize = textureNode->texture()->textureSize();
    if(m_fixHeight) {
        scaleSize.setHeight(boundingRect().height());
    }
    if(m_fixWidth) {
        scaleSize.setWidth(boundingRect().width());
    }
    if(!m_fixHeight && !m_fixWidth) {
        scaleSize = boundingRect().size();
    }
    const QSizeF size(texture->textureSize().scaled(scaleSize.toSize(), Qt::KeepAspectRatio));
    if(size != m_paintedSize) {
        m_paintedSize = size;
        Q_EMIT paintedSizeChanged();
    }
    QRect rect({ 0, 0 }, size.toSize());
    rect.moveCenter(boundingRect().toRect().center());
    textureNode->setRect(rect);
    m_renderPending = true;
    return textureNode;
}

void ThumbnailItem::classBegin()
{
    setupThumbnail();
    QQuickItem::classBegin();
}

QSGTextureProvider* ThumbnailItem::textureProvider() const
{
    if (!m_textureProvider) {
        m_textureProvider = new ThumbnailTextureProvider;
    }

    QSGTexture *texture = nullptr;
    if (m_thumbnail) {
        if (auto currentBuffer = m_thumbnail->currentBuffer()) {
            texture = currentBuffer->createTexture(this);
        }
    }
    m_textureProvider->updateTexture(texture);
    return m_textureProvider;
}

void ThumbnailItem::renderCompleted()
{
    if (m_thumbnail) {
        m_thumbnail->releaseBuffer(true);
        if (m_releaseQueue ) {
            m_releaseQueue->enqueue(m_thumbnail->takeRetiredBuffers(), true);
        }
    }
}

bool ThumbnailItem::shouldBeActive() const
{
    return isVisible() && window() && window()->isVisible();
}

void ThumbnailItem::setupThumbnail()
{
    thumbnail();
}

UkuiThumbnail::Thumbnail* ThumbnailItem::thumbnail()
{
    if (!m_thumbnail) {
        m_thumbnail = createThumbnail();
        connect(m_thumbnail, &UkuiThumbnail::Thumbnail::bufferUpdate, this, &ThumbnailItem::update);
    }
    return m_thumbnail;
}

qreal ThumbnailItem::paintedWidth() const
{
    return m_paintedSize.width();
}

qreal ThumbnailItem::paintedHeight() const
{
    return m_paintedSize.height();
}

void ThumbnailItem::onAfterRendering()
{
    if (!m_renderPending) {
        return;
    }
    m_renderPending = false;
    QMetaObject::invokeMethod(this, [this]() {
        renderCompleted();
    }, Qt::QueuedConnection);
}
