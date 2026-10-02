#include "cornerrotatinglabel.h"
#include <QPainter>

CornerRotatingLabel::CornerRotatingLabel(QWidget *parent)
    : QLabel(parent),
      m_rotationAngle(0),
      m_rotationOrigin(1.0f, 0.0f) // 右上角为原点(相对坐标)
{
    // 将动画分拆开来，不放在同一个类中去实现 防止动画状态记录混乱（切换音乐的时候会同时收到暂停/播放两个信号）
    m_startRotationAnimationGroup = new QSequentialAnimationGroup(this);
    m_rotateCounterAnimationGroup = new QSequentialAnimationGroup(this);
    m_resetAnimationGroup = new QSequentialAnimationGroup(this);
}

void CornerRotatingLabel::startRotationAnimation()
{
    if (m_startRotationAnimationGroup->state() == QAbstractAnimation::Running) {
        m_startRotationAnimationGroup->stop() ;
    }
    m_startRotationAnimationGroup->clear();
    QPropertyAnimation *rotateAnim = new QPropertyAnimation(this, "rotationAngle", this);
    rotateAnim->setDuration(500);
    rotateAnim->setEasingCurve(QEasingCurve::OutBack);
    rotateAnim->setStartValue(0);
    rotateAnim->setEndValue(-15);
    QPropertyAnimation *resetAnim = new QPropertyAnimation(this, "rotationAngle", this);
    resetAnim->setDuration(500);
    resetAnim->setEasingCurve(QEasingCurve::OutBounce);
    resetAnim->setStartValue(-15);
    resetAnim->setEndValue(0);
    m_startRotationAnimationGroup->addAnimation(rotateAnim);
    m_startRotationAnimationGroup->addAnimation(resetAnim);
    m_startRotationAnimationGroup->start();
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

void CornerRotatingLabel::rotateCounterClockwise()
{
    if (m_rotateCounterAnimationGroup->state() == QAbstractAnimation::Running
            || m_startRotationAnimationGroup->state() == QAbstractAnimation::Running) {
        return ;
    }
    m_rotateCounterAnimationGroup->clear();
    QPropertyAnimation *rotateAnim = new QPropertyAnimation(this, "rotationAngle", this);
    rotateAnim->setDuration(500);
    rotateAnim->setEasingCurve(QEasingCurve::OutBack);
    rotateAnim->setStartValue(m_rotationAngle);
    rotateAnim->setEndValue(m_rotationAngle - 15);
    m_rotateCounterAnimationGroup->addAnimation(rotateAnim);
    m_rotateCounterAnimationGroup->start();
}

void CornerRotatingLabel::resetRotation()
{
    if (m_resetAnimationGroup->state() == QAbstractAnimation::Running
            || m_startRotationAnimationGroup->state() == QAbstractAnimation::Running) {
        return ;
    }
    m_resetAnimationGroup->clear();
    QPropertyAnimation *resetAnim = new QPropertyAnimation(this, "rotationAngle", this);
    resetAnim->setDuration(500);
    resetAnim->setEasingCurve(QEasingCurve::OutBounce);
    resetAnim->setStartValue(m_rotationAngle);
    resetAnim->setEndValue(0);
    m_resetAnimationGroup->addAnimation(resetAnim);
    m_resetAnimationGroup->start();
}

void CornerRotatingLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPointF absOrigin(width(), 0);
    painter.save();
    painter.translate(absOrigin);
    painter.rotate(m_rotationAngle);
    if (!pixmap()->isNull()) {
        QRectF pixmapRect(-pixmap()->width(), 0, pixmap()->width() - 10, pixmap()->height() - 10);
        painter.drawPixmap(pixmapRect, *pixmap(), pixmap()->rect());
    }
    painter.restore();
}
