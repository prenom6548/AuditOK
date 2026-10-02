#ifndef SHUTDOWNANIMATION_H
#define SHUTDOWNANIMATION_H

#include <QWidget>
#include <QTimer>

class ShutdownAnimation : public QWidget
{
    Q_OBJECT

public:
    explicit ShutdownAnimation(QWidget *parent = nullptr);
    ~ShutdownAnimation();

    // 设置动画速度（定时器间隔）
    void setAnimationSpeed(int speedMs);

    // 设置动画大小
    void setAnimationSize(int width, int height);

    // 设置文字
    void setText(const QString &text);

    // 设置文字间距
    void setTextSpacing(int spacing);

    // 设置进度步长（控制动画快慢）
    void setProgressStep(float step);
    float progressStep() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer *m_pTimer;

    // 动画进度相关
    float m_animationProgress = 0.0f;  // 0.0 到 8.0
    int m_animationSpeed = 50;         // 定时器间隔（毫秒）
    float m_progressStep = 0.1f;       // 每次增加的进度

    // 动画尺寸
    int m_animationSize = 128;         // 控制胶囊动画大小

    // 文字相关
    QString m_text = "正在关机";
    QFont m_textFont;
    int m_textSpacing = 40;

    // 胶囊参数（硬编码，保持原效果）
    int m_capsuleWidth = 9;
    int m_capsuleHeight = 17;
    int m_capsuleSpacing = 5;
    int m_speedMultiplier = 7;         // 效果参数
    float m_gatherFactor = 0.7f;       // 聚拢因子
};

#endif // SHUTDOWNANIMATION_H

//使用方法
//ShutdownAnimation animation;
//animation.resize(1000, 1000);
//animation.setText("正在关机");
//animation.setAnimationSpeed(30);
//animation.setAnimationSize(100, 100);
////设置进度步长（控制动画快慢）
//animation.setProgressStep(0.3f);  // 默认是0.1f
////设置文字间距
//// animation.setTextSpacing(50);  // 默认是40
//animation.show();
