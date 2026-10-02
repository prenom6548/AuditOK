#ifndef TEXTUREGRABBER_H
#define TEXTUREGRABBER_H

#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGTexture>
#include <QSize>
#include <QtGlobal>

class TextureGrabber : public QObject
{
    Q_OBJECT
public:
    explicit TextureGrabber(QQuickWindow *sourceWindow, QObject *parent = nullptr);
    ~TextureGrabber();

    void setSourceItem(QQuickItem *item);
    void disconnectFromSource();
    void setEnabled(bool enabled);

private Q_SLOTS:
    void grabTexture();

Q_SIGNALS:
    void textureReady(quint32 textureId, const QSize &size, int comparisonKey);

private:
    QQuickWindow *m_sourceWindow = nullptr;
    QQuickItem *m_sourceItem = nullptr;
    int m_lastComparisonKey = -1;
    bool m_enabled = true;
};

#endif // TEXTUREGRABBER_H