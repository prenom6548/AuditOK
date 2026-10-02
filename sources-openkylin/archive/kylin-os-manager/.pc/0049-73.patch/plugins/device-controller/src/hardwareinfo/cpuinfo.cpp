#include "cpuinfo.h"
#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QApplication>

CpuInfo::CpuInfo() {

}

QString CpuInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        return model;
    } else if (value == "arch") {
        return arch;
    } else if (value == "cores") {
        return cores;
    } else if (value == "onlinecores") {
        return onlinecores;
    } else if (value == "threadprecore") {
        return threadprecore;
    } else if (value == "maxMHz") {
        return maxMHz;
    } else if (value == "L1dcache") {
        return L1dcache;
    } else if (value == "L1icache") {
        return L1icache;
    } else if (value == "L2cache") {
        return L2cache;
    } else if (value == "L3cache") {
        return L3cache;
    } else if (value == "Frequency") {
        return getFrequency();
    } else if (value == "UsageRate") {
        return getUsageRate();
    }else if (value == "Temprature") {
        return getTemprature();
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

QString CpuInfo::getFrequency() {
    // CPU 频率
    QProcess process;
    process.start("cpufreq-info");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n");

    QStringList currentFrequencies;
    for (const QString &line : lines) {
        if (line.contains("current CPU frequency")) {
            QStringList parts = line.split("is");
            if (parts.size() > 1) {
                QString currentFrequency = parts.at(1).trimmed().split(" ")[0];
                if (currentFrequency.contains(".")){
                    currentFrequency = QString::number(currentFrequency.toFloat()*1000);
                    currentFrequencies.append(currentFrequency);
                }else{
                    currentFrequencies.append(currentFrequency);
                }
            }
        }
    }
    double avgFreq;
    double totalFreq = 0;
    for (int i = 0; i < currentFrequencies.length(); ++i) {
        totalFreq += currentFrequencies[i].toDouble();
    }
    avgFreq = totalFreq / currentFrequencies.length();
    QString frequency = QString::number(avgFreq, 'f', 2) + " MHz";
    return frequency;
}

QString CpuInfo::getUsageRate() {
    // CPU 使用率
    QFile file("/proc/stat");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine(); // 读取第一行数据
        QStringList values = line.split(" ", QString::SkipEmptyParts);

        // 计算总的CPU时间
        quint64 totalCPUTime = 0;
        for (int i = 1; i < values.size(); ++i)
            totalCPUTime += values[i].toULongLong();

        // 计算CPU使用率
        quint64 idleTime = values[4].toULongLong(); // 空闲时间
        double cpuUsage = 100.0 - ((idleTime * 100.0) / totalCPUTime);
        // 输出CPU使用率
        QString usage = QString::number(cpuUsage, 'f', 2) + "%";
        file.close();
        return usage;
    } else {
        return "";
    }
}

QString CpuInfo::getTemprature() {
    // CPU 温度
    int i = 1;
    double temperature = 0;
    QString path = "/sys/class/thermal/thermal_zone" + QString::number(i) + "/temp";
    while (QFile(path).exists()) {
        QProcess process;
        process.start("cat", QStringList() << path);
        process.waitForFinished();
        QTextStream stream(&process);
        QString temperatureStr = stream.readAll().trimmed();
        temperature += temperatureStr.toFloat() / 1000;
        i++;
        path = "/sys/class/thermal/thermal_zone" + QString::number(i) + "/temp";
    }
    temperature = temperature / (i - 1);
    QString temp = QString::number(temperature, 'f', 1) + "°C";
    if(temp == "nan°C"){
        return QApplication::tr("This CPU model does not support temperature display function");
    }
    return temp;
}



