#include "data.h"

ContentMenu::ContentMenu(ContentType contentType, const QString &itemName, QObject *parent)
    : m_contentType(contentType), m_itemName(itemName), QObject(parent) {
    switch (contentType) {
    case ContentType::PlainText:
        m_iconName = "ukui-idtext-symbolic";
        break;
    case ContentType::RichText:
        m_iconName = "ukui-idtext-symbolic";
        break;
    case ContentType::Image:
        m_iconName = "image-x-generic-symbolic";
        break;
    case ContentType::Original:
        m_iconName = "";
        break;
    default:
        m_iconName = "";
        break;
    }
}

ContentMenu::ContentMenu(const ContentMenu &rhs)
{
    m_contentType = rhs.contentType();
    m_itemName = rhs.itemName();
}

ContentMenu &ContentMenu::operator=(const ContentMenu &rhs)
{
    m_contentType = rhs.contentType();
    m_itemName = rhs.itemName();
    return *this;
}

InMemoryImageProvider::InMemoryImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{

}

QImage InMemoryImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QImage image;
    if (m_images.find(id) == m_images.end()) {
        *size = QSize(0, 0);
        return image;
    }
    int width = m_images[id].width();
    int height = m_images[id].height();
    if(size) {
        *size = QSize(width, height);
    }

    image = m_images[id].scaled(requestedSize.width() > 0 ? requestedSize.width() : width, requestedSize.height() > 0 ? requestedSize.height() : height
                                , Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return image;
}

QList<QImage> InMemoryImageProvider::requestAllImage()
{
    qDebug()<<"requestAllImage"<<m_images.values();
    return m_images.values();
}

void InMemoryImageProvider::addImage(const QImage &image)
{
    QString id = "Image" + QString::number(m_imageId);
    m_imageId++;
    m_images[id] = image;
}

void InMemoryImageProvider::clear()
{
    m_images.clear();
    m_imageId = 0;
}

int InMemoryImageProvider::imageNumber() const
{
    return m_images.size();
}
