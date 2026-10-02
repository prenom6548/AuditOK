#include "shutdownanimation.h"
#include <QPainter>
#include <QApplication>
#include <QFontMetrics>
#include <QDebug>

ShutdownAnimation::ShutdownAnimation(QWidget *parent)
    : QWidget(parent)
{
    // 设置字体
    m_textFont = QApplication::font();
    m_textFont.setPointSize(20);
    m_textFont.setBold(true);
//    m_textFont.setFamily("Microsoft YaHei");

    // 创建定时器
    m_pTimer = new QTimer(this);
    connect(m_pTimer, &QTimer::timeout, this, [this]() {
        m_animationProgress += m_progressStep;
        if (m_animationProgress >= 8.0f) {
            m_animationProgress = 0.0f;
        }
        update();
    });

    m_pTimer->start(m_animationSpeed);
}

ShutdownAnimation::~ShutdownAnimation()
{
    if (m_pTimer) {
        delete m_pTimer;
    }
}

void ShutdownAnimation::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

//    // 绘制黑色半透明背景
//    painter.fillRect(rect(), QColor(0, 0, 0, 150));

    int centerX = width() / 2;
    int centerY = height() / 2;

    // 1. 绘制胶囊动画
    painter.save();
    painter.translate(centerX, centerY - 20);

    int radius = m_animationSize / 2;

    // 计算当前高亮点位置
    int currentBrightest = static_cast<int>(m_animationProgress) % 8;
    float progress = m_animationProgress - currentBrightest;  // 0.0~1.0

    // 聚拢效果
    int adjustedRadius = static_cast<int>(radius * m_gatherFactor);

    for (int i = 0; i < 8; i++) {
        painter.save();
        painter.rotate(45 * i);

        int distance = (i - currentBrightest + 8) % 8;

        // 更平滑的透明度计算（保持原来效果）
        int alpha = 0;
        switch (distance) {
        case 0:
            alpha = 255 - static_cast<int>(55 * progress);
            break;
        case 1:
            alpha = 200 + static_cast<int>(55 * progress);
            break;
        case 2:
            alpha = 180 + static_cast<int>(20 * progress);
            break;
        case 3:
            alpha = 140 + static_cast<int>(20 * progress);
            break;
        case 4:
            alpha = 100 + static_cast<int>(20 * progress);
            break;
        case 5:
            alpha = 60 + static_cast<int>(20 * progress);
            break;
        case 6:
            alpha = 30 + static_cast<int>(20 * progress);
            break;
        case 7:
            alpha = 10 + static_cast<int>(20 * progress);
            break;
        }

        alpha = qMin(qMax(alpha, 10), 255);

        // 绘制胶囊
        QColor color(255, 255, 255, alpha);
        painter.setBrush(color);
        painter.setPen(Qt::NoPen);

        // 使用调整后的半径，让胶囊更聚拢
        QRectF capsule(-m_capsuleWidth/2, -adjustedRadius + m_capsuleSpacing,
                      m_capsuleWidth, m_capsuleHeight);
        painter.drawRoundedRect(capsule, m_capsuleWidth/2, m_capsuleWidth/2);

        painter.restore();
    }
    painter.restore();

    // 2. 绘制"正在关机"文字
    painter.setFont(m_textFont);

    // 计算文字尺寸
    QFontMetrics fm(m_textFont);
    int textWidth = fm.horizontalAdvance(m_text);
    int textHeight = fm.height();

    // 计算文字位置（在图标下方）
    int textX = centerX - textWidth / 2;
    int textY = centerY + m_animationSize/2 + m_textSpacing;

    // 绘制文字阴影（增强可读性）
//    painter.setPen(QColor(0, 0, 0, 100));
//    painter.drawText(textX + 2, textY + 2, m_text);

    // 绘制主文字（白色）
    painter.setPen(Qt::white);
    painter.drawText(textX, textY, m_text);
}

void ShutdownAnimation::setAnimationSpeed(int speedMs)
{
    if (speedMs <= 0) {
        qWarning() << "动画速度必须大于0毫秒";
        return;
    }

    if (m_animationSpeed != speedMs) {
        m_animationSpeed = speedMs;
        m_pTimer->setInterval(speedMs);
    }
}

void ShutdownAnimation::setAnimationSize(int width, int height)
{
    int newSize = qMin(width, height);
    if (m_animationSize != newSize) {
        m_animationSize = newSize;
        update();
    }
}

void ShutdownAnimation::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;
        update();
    }
}


void ShutdownAnimation::setTextSpacing(int spacing)
{
    if (m_textSpacing != spacing) {
        m_textSpacing = spacing;
        update();
    }
}

void ShutdownAnimation::setProgressStep(float step)
{
    if (step <= 0.0f) {
        qWarning() << "进度步长必须大于0";
        return;
    }

    m_progressStep = step;
}

float ShutdownAnimation::progressStep() const
{
    return m_progressStep;
}
