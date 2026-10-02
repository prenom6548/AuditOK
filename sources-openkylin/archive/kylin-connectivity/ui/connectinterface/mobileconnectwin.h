#ifndef MOBILECONNECTWIN_H
#define MOBILECONNECTWIN_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QTimer>
#include <kysdk/applications/ktabbar.h>
#include "mobileqrcode.h"
#include "usbconnectwin.h"
#include "publicattributes.hpp"

using namespace kdk;

class MobileConnectWin : public QWidget
{
    Q_OBJECT
public:
    MobileConnectWin(QWidget *parent = nullptr);

    void setInterFaceCodeInfo(QString codeInfo = "");

    void setTheme(PublicAttributes::Theme theme);

    void changeFontSize(double fontSize);

Q_SIGNALS:
    void sigUSBconnectBtnClicked();

private:
    void setUsbUI(QString theme);

private:
    enum TabBarIndex {
        MobileQRcodeWin = 0,  // 0
        UsbConnWin,           // 1
    };

    MobileQRcode *m_mobileQRcode = nullptr;
    UsbConnectWin *m_usbConnWin = nullptr;
    QStackedWidget *m_stackedWin = nullptr;
    KTabBar *m_tabBar = nullptr;
    QWidget *m_comboBoxWin = nullptr;
    QLabel *m_spaceLab = nullptr;
    QLabel *m_choiceDescLab = nullptr;

    QString m_currentFlage = "";

    QString m_vvstr = tr("vivo");
    QString m_hwstr = tr("HUAWEI");
    QString m_xmstr = tr("Xiaomi");
    // QString m_sxstr = tr("SAMSUNG");
    QString m_oppostr = tr("OPPO");
    // QString m_otherstr = tr("Other");

public Q_SLOTS:
    void slotTabBarClicked(int index);
    void valueChanged(QString text);
};

#endif  // MOBILECONNECTWIN_H
