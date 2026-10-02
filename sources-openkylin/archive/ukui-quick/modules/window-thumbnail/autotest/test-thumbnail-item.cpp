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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QSGTextureProvider>
#include <QSignalSpy>
#include <QtTest>

#include "../core/thumbnail-item.h"

using namespace UkuiThumbnail;

namespace {

class FakeThumbnailBuffer : public ThumbnailBuffer
{
public:
    QSGTexture *createTexture(const QQuickItem *) override
    {
        ++createTextureCount;
        return nullptr;
    }

    bool releaseTextureResources() override
    {
        ++releaseTextureResourcesCount;
        return releaseTextureResourcesResult;
    }

    int createTextureCount = 0;
    int releaseTextureResourcesCount = 0;
    bool releaseTextureResourcesResult = true;
};

class OpenGlContextGuard final
{
public:
    bool create()
    {
        m_surface.setFormat(m_context.format());
        m_surface.create();
        return m_surface.isValid() && m_context.create() && m_context.makeCurrent(&m_surface);
    }

    ~OpenGlContextGuard()
    {
        m_context.doneCurrent();
    }

private:
    QOffscreenSurface m_surface;
    QOpenGLContext m_context;
};

class FakeThumbnail : public Thumbnail
{
    Q_OBJECT

public:
    explicit FakeThumbnail(QObject *parent = nullptr)
        : Thumbnail(parent)
    {
    }

    bool start() override
    {
        ++startCount;
        return startResult;
    }

    void stop() override
    {
        ++stopCount;
    }

    void setUuid(const QString &uuid) override
    {
        lastUuid = uuid;
        ++setUuidCount;
    }

    void setOutputId(const QString &outputId) override
    {
        lastOutputId = outputId;
        ++setOutputIdCount;
    }

    QSharedPointer<ThumbnailBuffer> currentBuffer() override
    {
        return current;
    }

    void releaseBuffer(bool release) override
    {
        releaseBufferCalls.append(release);
    }

    QList<QSharedPointer<ThumbnailBuffer>> takeRetiredBuffers() override
    {
        auto result = retired;
        retired.clear();
        return result;
    }

    QList<QSharedPointer<ThumbnailBuffer>> takeAllBuffers() override
    {
        auto result = all;
        all.clear();
        return result;
    }

    bool startResult = true;
    int startCount = 0;
    int stopCount = 0;
    int setUuidCount = 0;
    int setOutputIdCount = 0;
    QString lastUuid;
    QString lastOutputId;
    QList<bool> releaseBufferCalls;
    QSharedPointer<ThumbnailBuffer> current;
    QList<QSharedPointer<ThumbnailBuffer>> retired;
    QList<QSharedPointer<ThumbnailBuffer>> all;
};

class TestThumbnailItem : public ThumbnailItem
{
public:
    using ThumbnailItem::ThumbnailItem;
    using ThumbnailItem::active;
    using ThumbnailItem::classBegin;
    using ThumbnailItem::itemChange;
    using ThumbnailItem::onAfterRendering;
    using ThumbnailItem::releaseResources;
    using ThumbnailItem::textureProvider;
    using ThumbnailItem::thumbnail;

    FakeThumbnail *fakeThumbnail() const
    {
        return m_fakeThumbnail;
    }

protected:
    Thumbnail *createThumbnail() override
    {
        if (!m_fakeThumbnail) {
            m_fakeThumbnail = new FakeThumbnail(const_cast<TestThumbnailItem *>(this));
        }
        return m_fakeThumbnail;
    }

private:
    mutable FakeThumbnail *m_fakeThumbnail = nullptr;
};

}

class thumbnail_item_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndFixFlags();
    void classBeginAndUuidDriveThumbnailLifecycle();
    void visibilityAndActiveForwardToThumbnail();
    void textureProviderUsesCurrentBufferWhenAvailable();
    void sceneGraphStopQueuesAndReleasesAllBuffers();
};

void thumbnail_item_test::defaultsAndFixFlags()
{
    TestThumbnailItem item;
    QSignalSpy fixHeightSpy(&item, &ThumbnailItem::fixHeightChanged);
    QSignalSpy fixWidthSpy(&item, &ThumbnailItem::fixWidthChanged);

    QVERIFY(item.flags().testFlag(QQuickItem::ItemHasContents));
    QVERIFY(item.uuid().isEmpty());
    QCOMPARE(item.paintedWidth(), -1.0);
    QCOMPARE(item.paintedHeight(), -1.0);
    QVERIFY(!item.fixHeight());
    QVERIFY(!item.fixWidth());

    item.setFixHeight(false);
    item.setFixWidth(false);
    QCOMPARE(fixHeightSpy.count(), 0);
    QCOMPARE(fixWidthSpy.count(), 0);

    item.setFixHeight(true);
    item.setFixWidth(true);
    QVERIFY(item.fixHeight());
    QVERIFY(item.fixWidth());
    QCOMPARE(fixHeightSpy.count(), 1);
    QCOMPARE(fixWidthSpy.count(), 1);
}

void thumbnail_item_test::classBeginAndUuidDriveThumbnailLifecycle()
{
    TestThumbnailItem item;
    QSignalSpy uuidSpy(&item, &ThumbnailItem::uuidChanged);

    item.classBegin();
    QVERIFY(item.fakeThumbnail() != nullptr);

    item.setUuid(QStringLiteral("thumbnail-id"));
    QCOMPARE(item.uuid(), QStringLiteral("thumbnail-id"));
    QCOMPARE(item.fakeThumbnail()->lastUuid, QStringLiteral("thumbnail-id"));
    QCOMPARE(item.fakeThumbnail()->setUuidCount, 1);
    QCOMPARE(item.fakeThumbnail()->startCount, 0);
    QCOMPARE(uuidSpy.count(), 1);

    item.setUuid(QStringLiteral("thumbnail-id"));
    QCOMPARE(item.fakeThumbnail()->setUuidCount, 1);
    QCOMPARE(uuidSpy.count(), 1);

    item.onAfterRendering();
    QVERIFY(item.fakeThumbnail()->releaseBufferCalls.isEmpty());
}

void thumbnail_item_test::visibilityAndActiveForwardToThumbnail()
{
    TestThumbnailItem item;
    item.classBegin();

    item.active(false);
    QCOMPARE(item.fakeThumbnail()->stopCount, 1);

    item.active(true);
    QCOMPARE(item.fakeThumbnail()->startCount, 1);

    item.setVisible(false);
    QCOMPARE(item.fakeThumbnail()->stopCount, 2);

    item.setVisible(true);
    QCOMPARE(item.fakeThumbnail()->startCount, 1);
    QCOMPARE(item.fakeThumbnail()->stopCount, 3);
}

void thumbnail_item_test::textureProviderUsesCurrentBufferWhenAvailable()
{
    TestThumbnailItem item;
    item.classBegin();
    auto fakeBuffer = QSharedPointer<FakeThumbnailBuffer>::create();
    item.fakeThumbnail()->current = fakeBuffer;

    auto *provider = qobject_cast<ThumbnailTextureProvider *>(item.textureProvider());

    QVERIFY(provider != nullptr);
    QCOMPARE(fakeBuffer->createTextureCount, 1);
    QCOMPARE(provider->texture(), nullptr);
}

void thumbnail_item_test::sceneGraphStopQueuesAndReleasesAllBuffers()
{
    OpenGlContextGuard context;
    if (!context.create()) {
        QSKIP("An OpenGL context is unavailable on this test platform");
    }

    QQuickWindow window;
    TestThumbnailItem item;
    item.classBegin();
    item.setParentItem(window.contentItem());

    auto allBuffer = QSharedPointer<FakeThumbnailBuffer>::create();
    item.fakeThumbnail()->all.append(allBuffer);

    QVERIFY(QMetaObject::invokeMethod(&window, "sceneGraphAboutToStop", Qt::DirectConnection));

    QVERIFY(item.fakeThumbnail()->all.isEmpty());
    QCOMPARE(allBuffer->releaseTextureResourcesCount, 1);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    thumbnail_item_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-thumbnail-item.moc"
