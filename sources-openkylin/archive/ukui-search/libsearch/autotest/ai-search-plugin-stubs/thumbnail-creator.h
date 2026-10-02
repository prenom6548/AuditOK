#ifndef AI_SEARCH_PLUGIN_THUMBNAIL_CREATOR_STUB_H
#define AI_SEARCH_PLUGIN_THUMBNAIL_CREATOR_STUB_H

#include <QImage>
#include <QObject>
#include <QRunnable>
#include <QSize>

namespace UkuiSearch {

extern QImage g_thumbnailImage;
extern QString g_thumbnailReadyUrlOverride;

class ThumbnailCreator : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ThumbnailCreator(const QString &url, const QSize &targetSize, qreal dpr, QObject *parent = nullptr)
        : QObject(parent)
        , m_url(url)
    {
        Q_UNUSED(targetSize)
        Q_UNUSED(dpr)
    }

protected:
    void run() override
    {
        Q_EMIT ready(g_thumbnailReadyUrlOverride.isEmpty() ? m_url : g_thumbnailReadyUrlOverride, g_thumbnailImage);
    }

Q_SIGNALS:
    void ready(QString url, const QImage &image);

private:
    QString m_url;
};

}  // namespace UkuiSearch

#endif
