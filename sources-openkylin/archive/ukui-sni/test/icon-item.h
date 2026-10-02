#ifndef ICONITEM_H
#define ICONITEM_H

#include <QDebug>
#include <QObject>
#include <QQuickPaintedItem>
#include <QPainter>
#include <QPixmap>
#include <QIcon>

class IconItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QVariant icon READ getIcon WRITE setIcon)
    Q_PROPERTY(QVariant overlayIcon READ getOverlayIcon WRITE setOverlayIcon)
    Q_PROPERTY(bool isInverseColor READ getIsInverseColor WRITE setIsInverseColor)

public:
    IconItem(QQuickItem *parent = nullptr);

    QVariant getIcon() const;
    void setIcon(QVariant icon);

    QVariant getOverlayIcon() const;
    void setOverlayIcon(QVariant overlayIcon);

    bool getIsInverseColor();
    bool setIsInverseColor(bool isInverseColor);

protected:
    void paint(QPainter *painter) override;
private:
    bool isPixmapPureColor(const QPixmap &pixmap);

    QVariant m_icon;
    QVariant m_overlayIcon;
    bool m_isInverseColor = false;

    static QColor symbolicColor;
};

#endif // ICONITEM_H
