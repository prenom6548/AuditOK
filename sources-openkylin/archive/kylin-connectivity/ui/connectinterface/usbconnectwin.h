#ifndef USBCONNECTWIN_H
#define USBCONNECTWIN_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

#include "publicattributes.hpp"

class UsbConnectWin : public QWidget
{
    Q_OBJECT
public:
    UsbConnectWin(QWidget *parent = nullptr);
    ~UsbConnectWin();

    void setTheme(PublicAttributes::Theme theme);

    // 设置图片列表
    void setImageList(const QStringList &imageFileNameList);

    // 清空图片列表
    void clearImageList();

Q_SIGNALS:
    void sigUSBconnectBtnClicked();

public Q_SLOTS:
    // imageIndex属性变化槽函数
    void slotImageIndexChange(const QVariant &value);
    // 图片切换按钮点击;
    void onClicked();

private:
    // 滚动窗口的size
    static int ROLLING_WIN_W;
    static int ROLLING_WIN_H;

    // 用来作图片切换滑动效果，目前以透明度做为切换效果;
    QScrollArea *m_imagePlayWidget = nullptr;
    QWidget *m_imageWidget = nullptr;
    QHBoxLayout *m_imageLayout = nullptr;
    QPushButton *m_backImageBtn = nullptr;
    QPushButton *m_nextImageBtn = nullptr;
    QLabel *m_backImageLab = nullptr;
    QLabel *m_nextImageLab = nullptr;
    QPushButton *m_connectBtn = nullptr;

    // 图片切换动画类;
    QPropertyAnimation *m_opacityAnimation = nullptr;

    // 图片列表;
    QStringList m_imageFileNameList = {};

    // 当前显示图片index;
    int m_currentDrawImageIndex = 0;
};

#endif // USBCONNECTWIN_H
