#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#define private public  // hack complier
#define protected public

#include "mainwindow.h"
#include "add_printer_window.h"
#include "rename_printer_dialog.h"
#include "troubleshooting_dialog.h"
#include "base_notify_dialog.h"
#include "job_manager_window.h"
#include "smb_auth_info_dialog.h"
#include "property_window.h"
#include "choose_ppd_combo_box.h"
#include "check_uri_connected_helper.h"
#include "find_and_match_ppds.h"
#include "config_ip_address_dialog.h"
#include "job_properties_window.h"
#include "package_helper.h"
#include "ink_info_helper.h"
#include "page_info_helper.h"
#include "snmpwalk_browser.h"
#include "ukui_apt.h"
#include "upload_helper.h"
#include "cups_debug_logging_checkbox.h"
#include "ink_info_widget.h"
#include "upload_helper.h"
#include "property_itemwidget.h"
#include "daemonipcdbus.h"
#include "information_collector.h"
#include "system_notification.h"
#include "common.h"
#include "connect.h"
#include "base_info.h"
#include "uri.h"
#include "ukui_settings_monitor.h"
#include "horizontalorverticalmode.h"
#include "smbclient_helper.h"
#include "custom_about_dialog.h"
#include "custom_trans_widget.h"
#include "custom_check_box.h"
#include "custom_borderless_button.h"
#include "custom_label.h"
#include "custom_widget.h"
#include "custom_push_button.h"
#include "custom_message_box.h"
#include "waiting_dialog.h"
#include "log.h"
#include "daemonipcdbus.h"
#include "main_win_property_widget.h"

class PeonyPrintPicturesInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
       
    }

    static void TearDownTestSuite()
    {
        
    }

};

TEST_F(PeonyPrintPicturesInterfaceTest, a)
{
    checkCommand(QStringList());
    checkCommand(QStringList() << "debug");
    checkCommand(QStringList() << "show");
    checkCommand(QStringList() << "add");
    checkCommand(QStringList() << "add" << "aaa");
    checkCommand(QStringList() << "remove" << "aaa");
    checkCommand(QStringList() << "add_uri" << "aaa");
    checkCommand(QStringList() << "aaa" << "aaa");

    getUserName();
    isAdministratorUser();
    getDebVersion("kylin-printer");
    try_connect("10.41.120.168", 631, "aaa");
    checkLocalhostPort(631);
    devicePathCheck("aaa");
    encodeUrl("aaa");
    decodeUrl("aaa");
}

TEST_F(PeonyPrintPicturesInterfaceTest, b)
{
    BaseInfo::instance().getAppDisplayName();
    BaseInfo::instance().getServerAddress();
}

TEST_F(PeonyPrintPicturesInterfaceTest, c)
{
    Uri u("cups-pdf/:");
    u.protocolType = ProtocolType::LPD; u.getDeviceType();
    u.protocolType = ProtocolType::NONE; u.getDeviceType();
    u.protocolType = ProtocolType::LPD; u.getDeviceType();
    u.protocolType = ProtocolType::IPP; u.getDeviceType();
    u.protocolType = ProtocolType::IPPS; u.getDeviceType();
    u.protocolType = ProtocolType::HTTP; u.getDeviceType();
    u.protocolType = ProtocolType::HTTPS; u.getDeviceType();
    u.protocolType = ProtocolType::SOCKET; u.getDeviceType();
    u.protocolType = ProtocolType::DNSSD; u.getDeviceType();
    u.protocolType = ProtocolType::SERIAL; u.getDeviceType();
    u.protocolType = ProtocolType::PARALLEL; u.getDeviceType();
    u.protocolType = ProtocolType::SMB; u.getDeviceType();
    u.protocolType = ProtocolType::USB; u.getDeviceType();
    u.protocolType = ProtocolType::CUPS_PDF; u.getDeviceType();
    u.protocolType = ProtocolType::CUPS_BRF; u.getDeviceType();
    u.protocolType = ProtocolType::GUTENPRINT; u.getDeviceType();

}

TEST_F(PeonyPrintPicturesInterfaceTest, d)
{
    UkuiSettingsMonitor::instance().startTest(UkuiSettingsMonitor::TabletStatus::None);
}

TEST_F(PeonyPrintPicturesInterfaceTest, e)
{
    HorizontalOrVerticalMode w;
    w.rotationChanged("aaa");
    w.modeChanged(true);
}

TEST_F(PeonyPrintPicturesInterfaceTest, f)
{
    SMBAuthData data;
    data.host = "192.17.120.168";
    data.share = "aaa";
    data.workgroup = "WORKGROUP";
    data.username = "kylin";
    data.password = "kk123123";

    SmbClientHelper *h = new SmbClientHelper(data);
    h->browse(QString());
    h->browse("10.41.120.168");
    h->cconnect("10.41.120.168", "aaa");
    h->constructSmbUri("10.41.120.168", "aaa", "ccc", "ddd", "eee");
    h->constructSmbDevice("aaa");
    h->checkSmbServer("10.41.120.168");
    h->dowork();
    h->finished();

}

TEST_F(PeonyPrintPicturesInterfaceTest, g)
{
    CustomAboutDialog w;
    w.getAppVersion();
}

TEST_F(PeonyPrintPicturesInterfaceTest, h)
{
    CustomTransWidget w;
    auto s = QPaintEvent(QRect());
    w.paintEvent(&s);
    w.transChange(1);
}

TEST_F(PeonyPrintPicturesInterfaceTest, i)
{
    CustomCheckBox ww("aaa");
    CustomCheckBox w;
    w.setText("text");
    w.setFullText("text");
    w.setTextLimitShrink("text", 16);
    w.setTextLimitExpand("text");
    w.fullText();
    auto s = QPaintEvent(QRect());
    w.paintEvent(&s);
    w.elideText();
}

TEST_F(PeonyPrintPicturesInterfaceTest, j)
{
    CustomBorderlessButton w;
    w.setText("text");
    w.setFullText("text");
    w.setTextLimitShrink("text", 16);
    w.setTextLimitExpand("text");
    w.fullText();
    auto s = QPaintEvent(QRect());
    w.paintEvent(&s);
    w.elideText();
}

TEST_F(PeonyPrintPicturesInterfaceTest, k)
{
    CustomLabel w;
    w.setText("text");
    w.setFullText("text");
    w.setTextLimitShrink("text", 16);
    w.setTextLimitExpand("text");
    w.fullText();
    auto s = QPaintEvent(QRect());
    w.paintEvent(&s);
    w.elideText();
}

TEST_F(PeonyPrintPicturesInterfaceTest, l)
{
    CustomWidget w;
    w.setWidgetName("text");
    w.setIcon("text");
    w.handlePCMode();
    w.handlePADMode();
    auto s = QShowEvent();
    w.showEvent(&s);

}

TEST_F(PeonyPrintPicturesInterfaceTest, m)
{
    CustomPushButton w;
    w.setText("text");
    w.setFullText("text");
    w.setTextLimitShrink("text", 16);
    w.setTextLimitExpand("text");
    w.fullText();
    w.elideText();
}

TEST_F(PeonyPrintPicturesInterfaceTest, n)
{
    CustomMessageBox w;
    kdk::KMessageBox::StandardButtons b;
    CustomMessageBox ww(QIcon(), "aaa", "bbb", b);
    ww.addButton("aaa", kdk::KMessageBox::NoRole);
    ww.show();

}

TEST_F(PeonyPrintPicturesInterfaceTest, o)
{
    CustomMessageBox w;
    kdk::KMessageBox::StandardButtons b;
    CustomMessageBox ww(QIcon(), "aaa", "bbb", b);
    ww.addButton("aaa", kdk::KMessageBox::NoRole);
    ww.show();

}

TEST_F(PeonyPrintPicturesInterfaceTest, p)
{
    WaitingDialog w("aaa", 100);
    
}

TEST_F(PeonyPrintPicturesInterfaceTest, q)
{
    DeviceInformation info;
    devInfoPrint(info);
    info.deviceType = DeviceType::USB;
    devInfoPrint(info);

    DeviceInformation device;
    device.vendor = "aaa";
    device.model = "aaa";
    getPackagesNameFromHttp(device);

    getNameFromDeviceInformation(device);
    
}

TEST_F(PeonyPrintPicturesInterfaceTest, r)
{
    qInstallMessageHandler(logOutput);
    qDebug() << "aaa";
    qWarning() << "aaa";
    qCritical() << "aaa";
    qInfo() << "aaa";
}

TEST_F(PeonyPrintPicturesInterfaceTest, s)
{
    qInstallMessageHandler(myMessageOutput);
    qDebug() << "aaa";
    qInfo() << "aaa";
    qWarning() << "aaa";
    qCritical() << "aaa";
}

TEST_F(PeonyPrintPicturesInterfaceTest, t)
{
    DaemonIpcDbus::getInstance();

    DaemonIpcDbus s;
    s.daemonIsNotRunning();
    s.request(QStringList());
    s.request(QStringList() << "aaa");
}

TEST_F(PeonyPrintPicturesInterfaceTest, u)
{
    QWidget ww;
    SelectPpdDialog w(&ww);
    w.startSearch();
    w.searchClicked();
    w.sendOutPPDName();
    w.cancelAndClose();
}

TEST_F(PeonyPrintPicturesInterfaceTest, v)
{
    QWidget ww;
    MainWinPropertyWidget w(&ww);
    QLabel l;
    w.setFont(QFont(), &l);
    w.onEditPrinterNameBtnPressed();
    w.modifyCurrentPrinterName("aaa");
    w.updateWin("aaa");
}
