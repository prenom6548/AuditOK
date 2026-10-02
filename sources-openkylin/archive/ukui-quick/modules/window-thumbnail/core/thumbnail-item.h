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

#ifndef ABSTRACTTHUMBNAILITEM_H
#define ABSTRACTTHUMBNAILITEM_H

#include "thumbnail.h"
#include <QMetaObject>
#include <QMutex>
#include <QSGTextureProvider>
#include <QPointer>
#include <QSharedPointer>
#include <atomic>
#include "scene-graph-resource-release-queue.h"
namespace UkuiThumbnail
{
    class ThumbnailTextureProvider : public QSGTextureProvider
    {
        Q_OBJECT
    public:
        QSGTexture *texture() const override;
        void updateTexture(QSGTexture*);
    private:
        QPointer<QSGTexture> m_texture;
    };

    class ThumbnailItem : public QQuickItem
    {
        Q_OBJECT
        Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
        Q_PROPERTY(qreal paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
        Q_PROPERTY(qreal paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)
        Q_PROPERTY(bool fixHeight READ fixHeight WRITE setFixHeight NOTIFY fixHeightChanged)
        Q_PROPERTY(bool fixWidth READ fixWidth WRITE setFixWidth NOTIFY fixWidthChanged)

    public:
        explicit ThumbnailItem(QQuickItem *parent = nullptr);
        ~ThumbnailItem() override;
        QString uuid() const;
        void setUuid(const QString &uuid);

        qreal paintedWidth() const;
        qreal paintedHeight() const;
        void onAfterRendering();

        bool fixHeight() const;
        void setFixHeight(bool fixHeight);

        bool fixWidth() const;
        void setFixWidth(bool fixWidth);
    protected:
        void classBegin() override;
        bool isTextureProvider() const override { return true; }
        QSGNode* updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *) override;
        QSGTextureProvider* textureProvider() const override;

        virtual Thumbnail* createThumbnail() = 0;
        Thumbnail* thumbnail();
        void active(bool active);
        bool shouldBeActive() const;
    Q_SIGNALS:
        void uuidChanged();
        void paintedSizeChanged();
        void fixHeightChanged();
        void fixWidthChanged();
    private Q_SLOTS:
        void onWindowChanged(QQuickWindow *window);
    private:
        void setupThumbnail();
        void renderCompleted();
        Q_DISABLE_COPY(ThumbnailItem)
        Q_DISABLE_MOVE(ThumbnailItem)
        bool m_fixHeight {false};
        bool m_fixWidth {false};
        QString m_uuid;
        QSizeF m_paintedSize;
        UkuiThumbnail::Thumbnail *m_thumbnail{nullptr};
        bool m_renderPending {false};
        mutable ThumbnailTextureProvider *m_textureProvider{nullptr};
        QPointer<QQuickWindow> m_window;
        std::atomic_bool m_nodeNeedsRebuild{false};
        QPointer<SceneGraphResourceReleaseQueue> m_releaseQueue;
    };
}

#endif //THUMBNAILITEM_H
