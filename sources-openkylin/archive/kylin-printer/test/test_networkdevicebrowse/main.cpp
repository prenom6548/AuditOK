#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QThread>


#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QFile>

#include "device_information.h"
#include "test.h"
#include "network_device_monitor.h"
#include "lpinfo_browser.h"
#include "common.h"
#include "snmpwalk_browser.h"
#include "ukui_printer.h"
#include <iostream>


void test_NetworkDeviceMonitor()
{
    #ifdef REMOTE_PRINTER
        printf("error\n");
    #endif
    NetworkDeviceMonitor *p = NetworkDeviceMonitor::getInstance();
    Test *test = new Test();
    
    // p->sendTestinfo = true;
    
    // p->start();
    qDebug () << (p->getDeviceFromIP("172.17.128.28"));
}

void test_LpinfoBrowser()
{
    LpinfoBrowser *p =  LpinfoBrowser::getInstance();
    QObject::connect(p, &LpinfoBrowser::findLpinfoDevice, [=](const QString &ans){
        qDebug () << ans;
    });
    p->browse();
}

void test_snmpwalk()
{
    std::vector<std::string> buf;

    std::vector<HostNode> hosts = {
        { "172.17.30.111", "public" },
    };

    std::vector<OidNode> oids = {
        { "1.3.6.1.2.1.4.20.1.1" },
        { "1.3.6.1.2.1.4.20.1.3" },
        { "1.3.6.1.2.1.1.5" },
        { "1.3.6.1.2.1.43.9.2.1.8" },
        { "1.3.6.1.2.1.25.3.2.1.3" },
        { "1.3.6.1.2.1.43.5.1.1.17" },
    };
    const char *a = "1.3.6.1.2.1.1.5";
    const char *b = "1.3.6.1.2.1.25.3.2.1.3";
    std::vector<OidNode> oids1 = {
        { a },
    };
    std::vector<OidNode> oids2 = {
        { b },
    };
 
    snmpwalkBrowser_initialize(oids);
    int ans = snmpwalkBrowser_synchronous(buf, hosts, oids);
    for (int i = 0; i < buf.size(); i++) {
        printf("%s\n", buf[i].c_str());
    }

    // buf.clear();
    // snmpwalkBrowser_initialize(oids2);
    // ans = snmpwalkBrowser_synchronous(buf, hosts, oids2);
    // for (int i = 0; i < buf.size(); i++) {
    //     printf("%s\n", buf[i].c_str());
    // }
    qDebug() << buf.size();
}

int main(int argc, char **argv) {

    qRegisterMetaType<DeviceInformation>("DeviceInformation");
    qRegisterMetaType<DeviceInformation>("DeviceInformation&");
    QApplication app(argc, argv);
    qInstallMessageHandler(myMessageOutput);
    
    test_NetworkDeviceMonitor();

    // test_LpinfoBrowser();

    // test_snmpwalk();
// "usb://HP/LaserJet%20M506?serial=PHC8LC5BFW"
// "HP-LaserJet-M506"
// "hp/hp-laserjet_m506-ps.ppd.gz"
    // UkuiPrinterManager::getInstance().addPrinter("a", "b", "c", "d");

    return app.exec();
}
