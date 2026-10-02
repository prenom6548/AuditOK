#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QWidget>
#include <QSlider>
#include <QMouseEvent>
#include <QEvent>
#include <QCoreApplication>
#include <QDebug>

class CustomSlider : public QSlider
{
    Q_OBJECT

    friend class TestMainWindow;
public:
    explicit CustomSlider(QWidget *parent = nullptr);
    ~CustomSlider();

    void initStyle();
    void themeStyle(QString themeColor);

private:
    int m_value;       //滑动条的值
    bool m_mousePress; //鼠标是否按压

protected:
    void mousePressEvent(QMouseEvent *event) override; //单击

    void mouseReleaseEvent(QMouseEvent *event) override; //释放

signals:

    void sliderReleasedAt(int); //当鼠标释放时，抛出包含鼠标X坐标位置信息的信号

public slots:

    void changeThemeColorSlot(const QString &key);
};

#endif // CUSTOMSLIDER_H
