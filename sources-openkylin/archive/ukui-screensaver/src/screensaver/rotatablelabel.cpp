#include "rotatablelabel.h"
#include <QPainter>
#include <QPainterPath>

RotatingImageLabel::RotatingImageLabel(QWidget *parent)
    : QLabel(parent),
      m_currentAngle(0),
      m_isPaused(false)
{
    // 初始化动画
    m_animation = new QPropertyAnimation(this, "rotationAngle", this);
    m_animation->setDuration(2000); // 默认2秒/圈
    m_animation->setStartValue(0);
    m_animation->setEndValue(360);
    m_animation->setLoopCount(-1); // 无限循环

    // 初始化快速旋转动画
    m_quickAnimation = new QPropertyAnimation(this, "rotationAngle", this);
    m_quickAnimation->setDuration(1000); // 1秒完成
    m_quickAnimation->setEasingCurve(QEasingCurve::OutQuad);

    // 动画冲突处理
    connect(m_quickAnimation, &QPropertyAnimation::stateChanged, [this](){
        if (m_quickAnimation->state() == QPropertyAnimation::Running) {
            m_animation->pause();
        } else if (m_animation->state() == QPropertyAnimation::Paused) {
            m_animation->resume();
        }
    });

//    // 设置抗锯齿和高质量渲染
//    this->setAttribute(Qt::WA_TranslucentBackground);
//    this->setAlignment(Qt::AlignCenter);
}

void RotatingImageLabel::startRotation(int speedMsPerRound)
{
    if (m_pixmap.isNull()) return;

    m_animation->setDuration(speedMsPerRound);
    if (m_isPaused) {
        resumeRotation();
    } else {
        m_animation->start();
    }
    update();
}

void RotatingImageLabel::stopRotation()
{
    m_animation->stop();
    m_currentAngle = 0;
    m_isPaused = false;
    update();
}

void RotatingImageLabel::pauseRotation()
{
    if (m_animation->state() == QAbstractAnimation::Running) {
        m_animation->pause();
        m_isPaused = true;
    }
    update();
}

void RotatingImageLabel::resumeRotation()
{
    if (m_isPaused) {
        m_animation->resume();
        m_isPaused = false;
    }
}

void RotatingImageLabel::setRotationSpeed(int msPerRound)
{
    bool wasRunning = (m_animation->state() == QAbstractAnimation::Running);
    m_animation->setDuration(msPerRound);
    if (wasRunning) {
        m_animation->start(); // 重新启动以应用新速度
    }
}

void RotatingImageLabel::setImage(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

void RotatingImageLabel::setImage(const QString &filePath)
{
    m_pixmap.load(filePath);
    update();
}

int RotatingImageLabel::rotationAngle() const
{
    return m_currentAngle;
}

void RotatingImageLabel::setRotationAngle(int angle)
{
    m_currentAngle = angle;
    update(); // 触发重绘
}

void RotatingImageLabel::rotate90CCW()
{
    // 停止所有动画
    m_animation->stop();
    m_quickAnimation->stop();

    // 设置快速旋转动画
    int targetAngle = m_currentAngle - 90;
    m_quickAnimation->setStartValue(m_currentAngle);
    m_quickAnimation->setEndValue(targetAngle);
    m_quickAnimation->start();
}

void RotatingImageLabel::resetRotation()
{
    // 停止所有动画
    m_animation->stop();
    m_quickAnimation->stop();

    // 设置复位动画
    m_quickAnimation->setStartValue(m_currentAngle);
    m_quickAnimation->setEndValue(0);
    m_quickAnimation->setDuration(2000);
    m_quickAnimation->start();
}

void RotatingImageLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (m_pixmap.isNull()) {
        QLabel::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 计算绘制区域
    int side = qMin(width(), height());
    QRectF destRect((width() - side) / 2, (height() - side) / 2, side, side);

    // 设置圆形裁剪区域
    QPainterPath path;
    path.addEllipse(destRect);
    painter.setClipPath(path);

    // 旋转绘制
    painter.save();
    QPointF center = destRect.center();
    painter.translate(center);
    painter.rotate(m_currentAngle);
    painter.translate(-center);

    // 绘制图片
    painter.drawPixmap(destRect, m_pixmap, m_pixmap.rect());
    painter.restore();
}
