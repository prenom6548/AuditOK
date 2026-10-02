#ifndef CORNERROTATINGLABEL_H
#define CORNERROTATINGLABEL_H

#include <QLabel>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

class CornerRotatingLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(float rotationAngle READ rotationAngle WRITE setRotationAngle)
public:
    explicit CornerRotatingLabel(QWidget *parent = nullptr);

    // 旋转控制接口
    void startRotationAnimation(); // 开始右上角旋转动画
    void rotateCounterClockwise();
    void resetRotation();

signals:
    void rotationAnimationFinished(); // 动画完成信号

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    float rotationAngle() const;
    void setRotationAngle(float angle);

    QSequentialAnimationGroup *m_startRotationAnimationGroup;
    QSequentialAnimationGroup *m_rotateCounterAnimationGroup;
    QSequentialAnimationGroup *m_resetAnimationGroup;
    float m_rotationAngle;
    QPointF m_rotationOrigin; // 旋转原点(右上角)
};

#endif // CORNERROTATINGLABEL_H
