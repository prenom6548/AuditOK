#ifndef DrawIconLabel_H
#define DrawIconLabel_H

#include <QLabel>
#include <QPainterPath>

class DrawIconLabel : public QLabel
{
    Q_OBJECT
public:
    explicit DrawIconLabel(int radius = 10, QWidget *parent = nullptr);
    explicit DrawIconLabel(const QPixmap &pixmap, int radius = 10, QWidget *parent = nullptr);

    QPainterPath transformRectToPath(const QRect& rect,int radius,int corner) const;
    void setIcon(const QPixmap &pixmap);
    void setRadius(int radius);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_pixmap;
    int m_radius;
};

#endif // DrawIconLabel_H
