#ifndef SPLASHWINDOW_H
#define SPLASHWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QEvent>

class SplashWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SplashWindow(QWidget *parent = nullptr);
    ~SplashWindow();

    // 渐变隐藏窗口
    void fadeOut(int duration = 300);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void centerOnScreen();
    void syncToParent();

private:
    QLabel *m_logoLabel;
    QVBoxLayout *m_mainLayout;
    QPropertyAnimation *m_fadeAnimation;
};

#endif // SPLASHWINDOW_H
