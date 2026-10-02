#ifndef INFOBUTTON_H
#define INFOBUTTON_H
#include <QPushButton>
#include <QIcon>
#include <QGSettings>

class InfoButton : public QPushButton
{
    Q_OBJECT
public:
    explicit InfoButton(QWidget * parent = nullptr);
    void setIsOobe(bool isOobe) { m_isOobe = isOobe; }
    bool getIsOobe() const { return m_isOobe; }
    ~InfoButton() = default;

protected:
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void initUI();
    void updateForegroundColor();  // 添加函数声明

private:
    QColor m_backgroundColor;
    QColor m_foregroundColor;
    bool m_isOobe = false;  // 默认为非OOBE模式

    //监听主题的Gsettings
    QGSettings * m_styleGsettings = nullptr;

private Q_SLOTS:
    void onGSettingChaned(const QString &key);
};

#endif // INFOBUTTON_H
