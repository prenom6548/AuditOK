#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QThread>


#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QFile>

#include "common.h"
#include "snmpwalk_browser.h"

#include <iostream>

void test_snmpwalk()
{
    std::vector<std::string> buf;

    std::vector<HostNode> hosts = {
        { "172.17.7.14", "public" },
    };

    std::vector<OidNode> oids = {
        // { "1.3.6.1.2.1.4.20.1.1" },
        // { "1.3.6.1.2.1.4.20.1.3" },
        { "1.3.6.1.2.1.1.5" },
        // { "1.3.6.1.2.1.43.9.2.1.8" },
        // { "1.3.6.1.2.1.25.3.2.1.3" },
        // { "1.3.6.1.2.1.43.5.1.1.17" },
    };
    
    snmpwalkBrowser_initialize(oids);
    int ans = snmpwalkBrowser_synchronous(buf, hosts, oids);
    ans = snmpwalkBrowser_synchronous(buf, hosts, oids);
    ans = snmpwalkBrowser_synchronous(buf, hosts, oids);
    ans = snmpwalkBrowser_synchronous(buf, hosts, oids);
    for (int i = 0; i < buf.size(); i++) {
        printf("%s\n", buf[i].c_str());
    }
}

void test_backend_snmp()
{
    QString res = ::getRetFromCommand(QStringList{"/usr/lib/cups/backend/snmp", "172.17.7.143"});
    if (res.isEmpty()) {
        qDebug () << "not found!";
        return ;
    }
    qDebug() << qPrintable(res);
    res = res.split("\"").at(1);
    qDebug() << qPrintable(res);
}

int main(int argc, char **argv) {

    QApplication app(argc, argv);
    qInstallMessageHandler(myMessageOutput);

    // test_snmpwalk();

    test_backend_snmp();

    return app.exec();
}
