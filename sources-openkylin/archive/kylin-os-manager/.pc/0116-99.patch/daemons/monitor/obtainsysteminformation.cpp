#include "obtainsysteminformation.h"
#include <QProcess>
#include <QMap>
#include <QFile>
#include <QDebug>

#define SYSTEM_INFOMATION_FILE_PATH "/etc/kylin-os-manager/systemInformation.txt"

ObtainSystemInformation::ObtainSystemInformation()
{
    start();
}

void ObtainSystemInformation::start()
{
    QProcess proc;
    QList<SystemInfo> list;
    QByteArray tmp;
    QString cmd;
    int num = 0;
    int cycle = 0;

    proc.start("export LANGUAGE=en");
    proc.waitForFinished();

    cmd = "dmidecode -t system |grep Manufacturer";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    tmp = tmp.replace("\n", "");
    list.append(SystemInfo("整机制造商", tmp.split(':').last().replace(" ", "")));

    cmd = "dmidecode -t system |grep Product";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    tmp = tmp.replace("\n", "");
    list.append(SystemInfo("整机型号", tmp.split(':').last().replace(" ", "")));

    cmd = "dmidecode -t system |grep Serial";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    tmp = tmp.replace("\n", "");
    list.append(SystemInfo("序列号", tmp.split(':').last().replace(" ", "")));

    cmd = "python /usr/share/kylin-os-manager/script/architecture.py";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    tmp = tmp.replace("\n", "");
    tmp = tmp.replace("'", "");
    tmp = tmp.replace("(", "");
    tmp = tmp.replace(")", "");
    tmp = tmp.replace(",", "");
    tmp = tmp.replace(" ", "");
    list.append(SystemInfo("系统位数", tmp));

    cmd = "python /usr/share/kylin-os-manager/script/machine.py";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    tmp = tmp.replace("\n", "");
    tmp = tmp.replace("'", "");
    tmp = tmp.replace("(", "");
    tmp = tmp.replace(")", "");
    tmp = tmp.replace(",", "");
    tmp = tmp.replace(" ", "");
    list.append(SystemInfo("内核架构", tmp));

    cmd = "python /usr/share/kylin-os-manager/script/node.py";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    tmp = tmp.replace("\n", "");
    tmp = tmp.replace("'", "");
    tmp = tmp.replace("(", "");
    tmp = tmp.replace(")", "");
    tmp = tmp.replace(",", "");
    tmp = tmp.replace(" ", "");
    list.append(SystemInfo("主机名", tmp));

    cmd = "cat /etc/os-release |grep PRETTY_NAME";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    tmp = tmp.replace("\n", "");
    tmp = tmp.replace("\"", "");
    tmp = tmp.split('=').last();
    list.append(SystemInfo("操作系统版本", tmp));

    cmd = "python /usr/share/kylin-os-manager/script/release.py";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    tmp = tmp.replace("\n", "");
    tmp = tmp.replace("'", "");
    tmp = tmp.replace("(", "");
    tmp = tmp.replace(")", "");
    tmp = tmp.replace(",", "");
    tmp = tmp.replace(" ", "");
    list.append(SystemInfo("内核版本", tmp));

    cmd = "export LANGUAGE=en;lscpu | grep 'Model name:'";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    tmp = tmp.replace("\n", "");
    tmp = tmp.replace("Model name:", "");
    if (!tmp.isEmpty()) {
        while (tmp.at(0) == ' ') {
            tmp = tmp.mid(1);
        }
    }
    list.append(SystemInfo("处理器", tmp));

    cmd = "dmidecode -t memory";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    QString mem;
    num = 0;
    QString memSize = "";
    QString memType = "";
    QString memManufacturer = "";
    for (QString str : QString(tmp).split("\n")) {
        if (cycle == 0 && str.contains("Memory Device")) {
            cycle = 1;
            num++;
            if (num > 1) {
                mem += "  |  ";
            }
        }
        if (cycle == 1 && str.contains("Size:")) {
            memSize = str.split("Size:").last();
            memSize.replace("\n", "");
            memSize.replace(" ", "");
            if (memSize.contains("MB")) {
                memSize.chop(2);
                memSize = QString::number(memSize.toInt() / 1024) + "GB";
            }
        }
        if (cycle == 1 && str.contains("Manufacturer:")) {
            memManufacturer = str.split("Manufacturer:").last();
        }
        if (cycle == 1 && str.contains("Type:")) {
            memType = str.split("Type:").last();
        }
        if (cycle == 1 && str.contains("Configured Memory Speed:")) {
            cycle = 0;
            mem += memSize + memManufacturer + memType + str.split("Configured Memory Speed:").last();
        }
    }
    list.append(SystemInfo("内存", mem));

    cmd = "dmidecode -t baseboard |grep 'Product Name:'";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    tmp = tmp.replace("\n", "");
    list.append(SystemInfo("主板", tmp.split(':').last().replace(" ", "")));

    cmd = "lshw -C disk";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    QString disk;
    QString diskOne = "";
    num = 0;
    cycle = 0;
    for (QString str : QString(tmp).split("\n")) {
        if (cycle == 0 && str.contains("*-disk")) {
            cycle = 1;
            num++;
            if (num > 1) {
                disk += "  |  ";
            }
        }
        if (cycle == 1 && str.contains("product:")) {
            cycle = 2;
            diskOne = str.split("product:").last().simplified();
        }
        if (cycle == 2 && str.contains("size:")) {
            cycle = 0;
            QString deviceSize = str.split("(").last();
            if (!deviceSize.isEmpty()) {
                diskOne += "(" + deviceSize;
                disk += diskOne;
            }
        }
    }
    list.append(SystemInfo("硬盘", disk));

    cmd = "lshw -C display |grep 'product:'";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    QString dispaly;
    num = 0;
    for (QString byt : QString(tmp).split('\n')) {
        if (byt.isEmpty()) {
            continue;
        }
        num++;
        if (num > 1) {
            dispaly += "  |  ";
        }
        dispaly += byt.split("product:").last().simplified();
    }
    list.append(SystemInfo("显卡", dispaly));

    cmd = "hwinfo --monitor |grep Model:";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    QString monitor;
    num = 0;
    for (QString byt : QString(tmp).split('\n')) {
        if (byt.isEmpty()) {
            continue;
        }
        num++;
        if (num > 1) {
            monitor += "  |  ";
        }
        monitor += byt.split("Model:").last().simplified();
    }
    monitor = monitor.replace("\"", "");
    list.append(SystemInfo("显示器", monitor));

    cmd = "lshw -C network";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    QString net;
    QString netProduct;
    num = 0;
    cycle = 0;
    for (QString str : QString(tmp).split("\n")) {
        if (cycle == 0 && str.contains("*-network")) {
            cycle = 1;
            num++;
            netProduct = "";
            if (num > 1) {
                net += "  |  ";
            }
        }
        if (cycle == 1 && str.contains("product:")) {
            netProduct = str.split("product:").last().simplified();
        }
        if (cycle == 1 && str.contains("logical name:")) {
            cycle = 0;
            net += str.split("logical name:").last().simplified();
            if (!netProduct.isEmpty()) {
                net += "(" + netProduct + ")";
            }
        }
    }
    list.append(SystemInfo("网卡", net));

    cmd = "hwinfo --sound |grep Device:";
    proc.start("bash -c \"" + cmd + "\"");
    proc.waitForFinished();
    tmp = proc.readAllStandardOutput();
    QString audio;
    num = 0;
    for (QString byt : QString(tmp).split('\n')) {
        if (byt.isEmpty() || !byt.contains("\"")) {
            continue;
        }
        num++;
        if (num > 1) {
            audio += "  |  ";
        }
        QStringList tmpList = byt.split("\"");
        tmpList.removeAll("");
        audio += tmpList.last();
    }
    list.append(SystemInfo("声卡", audio));

    save(list);
}

void ObtainSystemInformation::save(QList<SystemInfo> list)
{
    if (list.isEmpty()) {
        qDebug() << "ObtainSystemInformation：整机信息为空";
        return;
    }
    QFile file(SYSTEM_INFOMATION_FILE_PATH);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qDebug() << "ObtainSystemInformation：打开文件失败";
        return;
    }
    qDebug() << "ObtainSystemInformation：数据行数 " << list.length();
    for (SystemInfo info : list) {
        QString left = info.first;
        //        if (left.length() < 4) {
        //            left += "\t";
        //        }
        QString w = left + "\t" + info.second + "\n";
        file.write(w.toLocal8Bit());
    }
    qDebug() << "ObtainSystemInformation：写入完成";
    file.close();
}
