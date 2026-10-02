#ifndef ITEMICONPROVIDER_H
#define ITEMICONPROVIDER_H

#include <QSize>
#include <QQuickImageProvider>

class ItemIconProvider : public QQuickImageProvider
{
public:
    ItemIconProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
    static QPixmap getPixmap(const QString &id, QSize *size, const QSize &requestedSize);

private:
    static void loadPixmap(const QString &id, QPixmap &pixmap);
    static void loadDefault(QPixmap &pixmap);
    static bool loadPixmapFromUrl(const QUrl &url, QPixmap &pixmap);
    static bool loadPixmapFromPath(const QString &path, QPixmap &pixmap);
    static bool loadPixmapFromTheme(const QString &name, QPixmap &pixmap);
    static bool loadPixmapFromXdg(const QString &name, QPixmap &pixmap);

    static QSize m_defaultSize;
};

#endif // ITEMICONPROVIDER_H
