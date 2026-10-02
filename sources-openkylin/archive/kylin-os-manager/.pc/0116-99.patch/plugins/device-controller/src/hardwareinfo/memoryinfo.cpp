#include "memoryinfo.h"
#include <QProcess>
#include <QStringList>

MemoryInfo::MemoryInfo() {

}

QString MemoryInfo::getValue(QString value) {
    if (value == "locator") {
        return locator;
    } else if (value == "totalwidth") {
        return totalwidth;
    } else if (value == "size") {
        return size;
    } else if (value == "serial") {
        return serial;
    } else if (value == "verdor") {
        return verdor;
    } else if (value == "datawidth") {
        return datawidth;
    } else if (value == "type") {
        return type;
    } else if (value == "speed") {
        return speed;
    } else if (value == "configuredspeed") {
        return configuredspeed;
    } else if (value == "banklocator") {
        return banklocator;
    } else if (value == "partnumber") {
        return partnumber;
    } else if (value == "displayname") {
        return displayname;
    } else if (value == "Memory Usage") {
        return getUsage();
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

QString MemoryInfo::getUsage() {
    QProcess process;
    process.start("cat", QStringList() << "/proc/meminfo");
    process.waitForFinished();

    if (process.exitCode() == 0) {
        QString result = process.readAllStandardOutput();
        QStringList lines = result.split('\n');

        double totalMemory = 0.0;
        double freeMemory = 0.0;

        for (const QString &line: lines) {
            if (line.startsWith("MemTotal:")) {
                totalMemory = QString(QString(line.split(":")[1]).split("kB")[0]).trimmed().toDouble();
            } else if (line.startsWith("MemAvailable:")) {
                freeMemory = QString(QString(line.split(":")[1]).split("kB")[0]).trimmed().toDouble();
            }
        }

        if (totalMemory > 0) {
            double usedMemory = totalMemory - freeMemory;
            double memoryUsage = (usedMemory / totalMemory) * 100.0;
            QString temp = QString::number(memoryUsage, 'f', 2) + "%";
            return temp;
        }
    }
}
