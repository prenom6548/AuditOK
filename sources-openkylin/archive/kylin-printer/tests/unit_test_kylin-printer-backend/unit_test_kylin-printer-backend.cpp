#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#define private public  // hack complier
#define protected public

#include <QJsonObject>

#include "launch_printer.h"
#include "tray_win.h"
#include "event_notify_monitor.h"
#include "device_information.h"
#include "pop_window_manager.h"
#include "find_and_match_ppds.h"
#include "device_map.h"
#include "device_monitor.h"
#include "network_device_detector.h"
#include "network_device_monitor.h"

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
    LaunchPrinter::getInstance();
    LaunchPrinter *lp = new  LaunchPrinter();
    lp->add("/devices/pci0000:00/0000:00:14.0/usb1/1-6");
    lp->remove("/devices/pci0000:00/0000:00:14.0/usb1/1-6");
    lp->add_uri("/devices/pci0000:00/0 000:00:14.0/usb1/1-6");
    lp->findPrePrinter();
    lp->actionInvokeSlot(1, "kylin-printer-add-printer:");
    lp->actionInvokeSlot(1, "kylin-printer-print-test-page:");
    DeviceInformation info;
    info.displayName = "unit_test_printer";
    lp->actionInvokeSlot(1, QString("kylin-printer-print-test-page:%1").arg(info.serialize()));
    lp->actionInvokeSlot(1, "kylin-printer-print-test-page-failed:");

}

TEST_F(PeonyPrintPicturesInterfaceTest, b)
{
    QStringList ls;
    ls << "aaa" << "bbb";
    TrayWin *w = new TrayWin();
    w->m_minimizeAction->triggered(true);
    w->m_maximizeAction->triggered(true);
    w->m_maximizeAction->triggered(true);
    w->showNotify(ls);
    delete w;
}

TEST_F(PeonyPrintPicturesInterfaceTest, c)
{
    EventNotifyMonitor *notify = new EventNotifyMonitor();
    notify->detectStart();
    notify->getAndSendNotify();
    notify->renewSubscription();
    notify->cancelSubscription();
    notify->getNotifications();
    notify->insertUpdatePrinterName("unit_test_printer");
    // notify->~EventNotifyMonitor();
}

TEST_F(PeonyPrintPicturesInterfaceTest, d)
{
    DeviceInformation info;
    info.displayName = "unit_test_printer";
    PopWindowManager *w = new PopWindowManager();
    w->addNewPrinterDevice(info);
    w->getDevInfo("aaa");
    w->modifyInfoList("aaa");
    w->showNext();
    // w->showPopWindow(info);
    delete w;
}

TEST_F(PeonyPrintPicturesInterfaceTest, e)
{
    DeviceInformation info;
    info.displayName = "unit_test_printer";
    PopWindowManager *w = new PopWindowManager();
    w->addNewPrinterDevice(info);
    w->getDevInfo("aaa");
    w->modifyInfoList("aaa");
    w->showNext();
    // w->showPopWindow(info);
    delete w;
}

TEST_F(PeonyPrintPicturesInterfaceTest, f)
{
    DeviceInformation info;
    info.displayName = "unit_test_printer";
    PopWindowManager *w = new PopWindowManager();
    w->addNewPrinterDevice(info);
    w->getDevInfo("aaa");
    w->modifyInfoList("aaa");
    w->showNext();
    // w->showPopWindow(info);
    delete w;
}

TEST_F(PeonyPrintPicturesInterfaceTest, g)
{
    DeviceMap deviceMap;
    deviceMap.jsonReadFromOldGsettings(QJsonObject());
    DeviceMap::DevInfo info;
    deviceMap.writeDevInfoToJson(info);
    deviceMap.doPrinterRemove("unit_test_printer");
    deviceMap.doPrinterAdd("unit_test_printer");

}

TEST_F(PeonyPrintPicturesInterfaceTest, h)
{
    DeviceMonitorBackend::getInstance();
    DeviceMonitorBackend::getInstance()->usbDeviceIdentify("/devices/pci0000:00/0000:00:11.0/0000:02:01.0/usb1/1-1");
    DeviceMonitorBackend::getInstance()->doDeviceAdd("/devices/pci0000:00/0000:00:11.0/0000:02:01.0/usb1/1-1");
    DeviceMonitorBackend::getInstance()->doDeviceRemove("/devices/pci0000:00/0000:00:11.0/0000:02:01.0/usb1/1-1");

}

TEST_F(PeonyPrintPicturesInterfaceTest, i)
{
    NetworkDeviceDetector::getDeviceInfoFromHost("10.41.120.168", 631, ProtocolType::NONE);
    NetworkDeviceDetector::getInstance();
    DeviceInformation info;
    NetworkDeviceDetector::getInstance()->detectNetWorkNode(info);
    NetworkDeviceDetector::getInstance()->detectVendorAndModel(info);
    NetworkDeviceDetector::getInstance()->detectUUID(info);
    DeviceInformation iinfo;
    iinfo.protocolType = ProtocolType::DNSSD;
    NetworkDeviceDetector::getInstance()->detectUUID(iinfo);
    NetworkDeviceDetector::getInstance()->detectSerial(info);
    NetworkDeviceDetector::getInstance()->setUri(info, ProtocolType::NONE);
    NetworkDeviceDetector::getInstance()->setUri(info, ProtocolType::SOCKET);
    NetworkDeviceDetector::getInstance()->setUri(info, ProtocolType::HTTP);
    NetworkDeviceDetector::getInstance()->setUri(info, ProtocolType::IPP);
    NetworkDeviceDetector::getInstance()->probe(info, "10.41.120.168", 631, ProtocolType::NONE);
    NetworkDeviceDetector::getInstance()->probe(info, QString(), 631, ProtocolType::NONE);
    NetworkDeviceDetector::getInstance()->probe(info, "10.41.120.168", 515, ProtocolType::NONE);
    QList<DeviceInformation> list;
    list << info;
    NetworkDeviceDetector::getInstance()->probeIpp(list, "10.41.120.168", 631);
}

TEST_F(PeonyPrintPicturesInterfaceTest, j)
{
    NetworkDeviceMonitor::getDeviceFromIP("10.41.120.168", 631, ProtocolType::SMB);
    NetworkDeviceMonitor::getDeviceFromIP("10.41.120.168", 631, ProtocolType::LPD);
    NetworkDeviceMonitor::getDeviceFromIP("10.41.120.168", 631, ProtocolType::NONE);
    NetworkDeviceMonitor::getInstance();
}


