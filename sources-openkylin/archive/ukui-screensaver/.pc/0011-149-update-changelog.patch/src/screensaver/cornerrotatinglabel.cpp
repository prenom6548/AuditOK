#include "cornerrotatinglabel.h"
#include <QPainter>

CornerRotatingLabel::CornerRotatingLabel(QWidget *parent)
    : QLabel(parent),
      m_rotationAngle(0),
      m_rotationOrigin(1.0f, 0.0f) // 右上角为原点(相对坐标)
{
    m_animationGroup = new QSequentialAnimationGroup(this);

    // 逆时针旋转60度
    QPropertyAnimation *rotateAnim = new QPropertyAnimation(this, "rotationAngle", this);
    rotateAnim->setDuration(500);
    rotateAnim->setEasingCurve(QEasingCurve::OutBack);
    rotateAnim->setStartValue(0);
    rotateAnim->setEndValue(-15); // 逆时针

    // 还原旋转
    QPropertyAnimation *resetAnim = new QPropertyAnimation(this, "rotationAngle", this);
    resetAnim->setDuration(500);
    resetAnim->setEasingCurve(QEasingCurve::OutBounce);
    resetAnim->setStartValue(-15);
    resetAnim->setEndValue(0);

    m_animationGroup->addAnimation(rotateAnim);
    m_animationGroup->addAnimation(resetAnim);

    connect(m_animationGroup, &QSequentialAnimationGroup::finished,
            this, &CornerRotatingLabel::rotationAnimationFinished);
}

void CornerRotatingLabel::startRotationAnimation()
{
    if (m_animationGroup->state() == QAbstractAnimation::Running) {
        m_animationGroup->stop();
    }
    m_animationGroup->start();
}

float CornerRotatingLabel::rotationAngle() const
{
    return m_rotationAngle;
}

void CornerRotatingLabel::setRotationAngle(float angle)
{
    m_rotationAngle = angle;
    update(); // 触发重绘
}

void CornerRotatingLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 计算绝对旋转原点(右上角)
    QPointF absOrigin(width(), 0);

    painter.save();

    // 将坐标系平移到旋转原点
    painter.translate(absOrigin);

    // 应用旋转
    painter.rotate(m_rotationAngle);

    if (!pixmap()->isNull()) {
        QRectF pixmapRect(-pixmap()->width(), 0, pixmap()->width() - 10, pixmap()->height() - 10);
        painter.drawPixmap(pixmapRect, *pixmap(), pixmap()->rect());
    }

    painter.restore();
}
