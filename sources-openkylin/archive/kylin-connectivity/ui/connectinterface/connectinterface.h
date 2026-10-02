#ifndef CONNECTINTERFACE_H
#define CONNECTINTERFACE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <kysdk/applications/ktabbar.h>
#include "searchdevicewin.h"
#include "mobileconnectwin.h"
#include "devicecodewidget.h"
#include "backgroundwin.h"
#include "publicattributes.hpp"

using namespace kdk;

class ConnectInterface : public QWidget
{
    Q_OBJECT
public:
    ConnectInterface(QWidget *parent = nullptr);

    void setTheme(PublicAttributes::Theme theme);
    void setInterFaceCodeInfo(const QString wifiInfo);
    void setItemInfo(const QMap<QString, KCommon::DiscoveryDeviceInfo> &map);
    void showLoad();
    void setSearchBtnState(bool state);
    void changeFontSize(double fontSize);

Q_SIGNALS:
    void sigBackBtnClicked();
    void sigUSBconnectBtnClicked();
    void sigConnectAddress(QString address);
    void sigContinueSearch();
    void sigItemInfo(QString);

private:
    void initUI();

private:
    enum TabBarIndex {
        NearbyDevice = 0, // 0
        ConnectPhone,     // 1
        DeviceCode,       // 2
    };

    KTabBar *m_tabBar = nullptr;
    QPushButton *m_backBtn = nullptr;
    BackgroundWin *m_backgroundWin = nullptr;
    QStackedWidget *m_stackedWin = nullptr;
    SearchDeviceWin *m_searchWin = nullptr;
    MobileConnectWin *m_mobileConnectWin = nullptr;
    DeviceCodeWidget *m_deviceCodeWidget = nullptr;

public Q_SLOTS:
    void slotTabBarClicked(int index);
};

#endif // CONNECTINTERFACE_H
