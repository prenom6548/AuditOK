#include "texturegrabber.h"
#include <QSGTexture>
#include <QSGTextureProvider>
#include <QDebug>

TextureGrabber::TextureGrabber(QQuickWindow *sourceWindow, QObject *parent)
    : QObject(parent)
    , m_sourceWindow(sourceWindow)
{
    connect(sourceWindow, &QQuickWindow::afterRendering,
            this, &TextureGrabber::grabTexture,
            Qt::DirectConnection);
}

TextureGrabber::~TextureGrabber()
{
    disconnectFromSource();
}

void TextureGrabber::disconnectFromSource()
{
    m_enabled = false;
    if (m_sourceWindow) {
        m_sourceWindow->disconnect(this);
        m_sourceWindow = nullptr;
    }
}

void TextureGrabber::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

void TextureGrabber::setSourceItem(QQuickItem *item)
{
    m_sourceItem = item;
}

void TextureGrabber::grabTexture()
{
    if (!m_enabled || !m_sourceItem) {
        return;
    }

    // 再次检查，防止 DirectConnection 的同步调用在检查之后进入
    if (!m_sourceItem) {
        return;
    }

    QSGTextureProvider *provider = m_sourceItem->textureProvider();
    if (!provider) {
        return;
    }

    QSGTexture *texture = provider->texture();
    if (!texture) {
        return;
    }

    int key = texture->comparisonKey();
    static int frameCount = 0;
    frameCount++;
    bool shouldForceUpdate = (frameCount % 5 == 0);

    if (key == m_lastComparisonKey && !shouldForceUpdate) {
        return;
    }
    m_lastComparisonKey = key;

    GLuint textureId = (GLuint)texture->textureId();
    QSGTexture::NativeTexture nativeTex = texture->nativeTexture();
    QSize size = texture->textureSize();

    if ((!nativeTex.object && textureId <= 0) || size.isEmpty()) {
        m_lastComparisonKey = -1;
        return;
    }

    Q_EMIT textureReady(textureId, size, key);
}
