#ifndef DATA_H
#define DATA_H

#include <QObject>
#include <QQuickImageProvider>
#include <QImage>

class ContentMenu : public QObject {
    Q_OBJECT
    Q_PROPERTY(ContentType contentType READ contentType CONSTANT)
    Q_PROPERTY(QString itemName READ itemName CONSTANT)
    Q_PROPERTY(QString iconName READ iconName CONSTANT)

public:
    enum ContentType {
        PlainText = 0,
        RichText,
        Image,
        Original,
    };
    Q_ENUM(ContentType);

    ContentMenu() = default;
    ContentMenu(ContentType contentType, const QString &itemName, QObject *parent = nullptr);
    ContentMenu(const ContentMenu &rhs);
    ContentMenu& operator=(const ContentMenu &rhs);
    virtual ~ContentMenu() = default;

    ContentType contentType() const { return m_contentType; }
    QString itemName() const { return m_itemName; }
    QString iconName() const { return m_iconName; }

signals:

private:
    ContentType m_contentType;
    QString m_itemName;
    QString m_iconName;
};

Q_DECLARE_METATYPE(ContentMenu)

class InMemoryImageProvider : public QQuickImageProvider
{
public:
    InMemoryImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    QList<QImage> requestAllImage();
    void addImage(const QImage &image);
    void clear();
    int imageNumber() const;

private:
    int m_imageId = 0;
    QMap<QString, QImage> m_images; // 存储图片ID与QImage的映射
};

#endif // DATA_H
