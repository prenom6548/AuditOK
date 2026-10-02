#ifndef ROTATINGIMAGELABEL_H
#define ROTATINGIMAGELABEL_H

#include <QLabel>
#include <QPropertyAnimation>
#include <QPixmap>

class RotatingImageLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int rotationAngle READ rotationAngle WRITE setRotationAngle)
public:
    explicit RotatingImageLabel(QWidget *parent = nullptr);

    // 控制接口
    void startRotation(int speedMsPerRound = 2000); // 开始旋转(默认2秒/圈)
    void stopRotation();                           // 停止并重置角度
    void pauseRotation();                          // 暂停旋转
    void resumeRotation();                         // 从暂停恢复
    void setRotationSpeed(int msPerRound);         // 设置旋转速度

    // 图片设置
    void setImage(const QPixmap &pixmap);
    void setImage(const QString &filePath);

    int rotationAngle() const;
    void setRotationAngle(int angle);

    void rotate90CCW();          // 1秒逆时针旋转90度
    void resetRotation();        // 复位到0度

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPropertyAnimation *m_animation;
    QPropertyAnimation *m_quickAnimation;  // 用于快速旋转
    QPixmap m_pixmap;
    int m_currentAngle;
    bool m_isPaused;
};

#endif // ROTATINGIMAGELABEL_H
