#ifndef CONNECTEDWIN_H
#define CONNECTEDWIN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "publicattributes.hpp"
#include "backgroundwin.h"
#include "disconnectbutton.h"

class ConnectedWin : public QWidget
{
    Q_OBJECT
public:
    enum BtnType {
        Screen = 0,  // 投屏
        ExitScreen,  // 退出投屏
        Disconnect,  // 断开连接
    };
    ConnectedWin(QWidget *parent = nullptr);

    void setTheme(PublicAttributes::Theme theme);
    void setDeviceName(QString deviceName);
    void setDeviceType(PublicAttributes::DeviceType type, bool isInitiator = false);
    void addWidget(QWidget *win);
    void restoreScreenButton();
    void changeScreenButton(bool isShow);
    void changeFontSize(double fontSize);

Q_SIGNALS:
    void sigBtnClicked(ConnectedWin::BtnType type);

public Q_SLOTS:
    void onClicked();

private:
    QLabel *m_deviceIcon = nullptr;
    QLabel *m_deviceName = nullptr;
    QLabel *m_stateIcon = nullptr;
    QLabel *m_connectState = nullptr;
    QPushButton *m_screenBtn = nullptr;
    DisconnectButton *m_disconnectBtn = nullptr;
    BackgroundWin *m_backgroundWin = nullptr;
    QHBoxLayout *m_layout = nullptr;

    bool m_isScreenBtnClicked = false;
    QString m_ScreenText = "";
};

#endif  // CONNECTEDWIN_H
