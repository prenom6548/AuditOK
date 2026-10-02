#ifndef SHADOWBORDERITEM_H
#define SHADOWBORDERITEM_H

#include <QPainter>
#include <QPainterPath>
#include <QQuickPaintedItem>

class ShadowBorderItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal verticalOffset READ verticalOffset WRITE setVerticalOffset
                   NOTIFY verticalOffsetChanged)
    Q_PROPERTY(qreal keyRadius READ keyRadius WRITE setKeyRadius NOTIFY
                   keyRadiusChanged)

public:
    explicit ShadowBorderItem(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    QColor color() const;
    void setColor(const QColor &color);
    qreal verticalOffset() const;
    void setVerticalOffset(qreal offset);
    qreal keyRadius() const;
    void setKeyRadius(qreal radius);

signals:
    void colorChanged();
    void verticalOffsetChanged();
    void keyRadiusChanged();

private:
    void calculateBorderRadius();
    void drawBorder(QPainter *painter, bool isTop);

private:
    QColor borderColor_;
    qreal outerRadius_;
    qreal innerRadius_;
    qreal verticalOffset_;
    qreal keyRadius_;
};

#endif // SHADOWBORDERITEM_H
