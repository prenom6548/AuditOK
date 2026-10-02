#ifndef ADAPTIVELABEL_H
#define ADAPTIVELABEL_H

#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QtMath>

class AdaptiveLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AdaptiveLabel(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);

    void setScale(float scale);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap originalPixmap;
    float m_scale;
};

#endif // ADAPTIVELABEL_H

