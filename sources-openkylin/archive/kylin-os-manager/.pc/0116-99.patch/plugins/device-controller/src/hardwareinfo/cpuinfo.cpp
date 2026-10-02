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
    } else if (value == "L4cache") {
        return L4cache;
    } else if (value == "Frequency") {
        return getFrequency();
    } else if (value == "UsageRate") {
        return getUsageRate();
    } else if (value == "Temprature") {
        return getTemprature();
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

QString CpuInfo::getFrequency() {
    // CPU 频率
    QProcess proc3;
    QString commond3 = "cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    proc3.start(commond3);
    proc3.waitForFinished();
    QString outPut3 = proc3.readAllStandardOutput().trimmed();
    proc3.close();
    if (outPut3 == "powersave") {
        QProcess process;
        process.start("cpufreq-info");
        process.waitForFinished();

        QString output = process.readAllStandardOutput();
        QStringList lines = output.split("\n");

        QStringList currentFrequencies;
        for (const QString &line: lines) {
            if (line.contains("current CPU frequency")) {
                QStringList parts = line.split("is");
                if (parts.size() > 1) {
                    QString currentFrequency = parts.at(1).trimmed().split(" ")[0];
                    if (currentFrequency.contains(".")) {
                        currentFrequency = QString::number(currentFrequency.toFloat() * 1000);
                        currentFrequencies.append(currentFrequency);
                    } else {
                        currentFrequencies.append(currentFrequency);
                    }
                }
            }
        }
        double min = 0;
        for (int i = 0; i < currentFrequencies.length(); ++i) {
            if (min > currentFrequencies[i].toDouble()) {
                min = currentFrequencies[i].toDouble();
            }
        }
        QString frequency = QString::number(min, 'f', 2) + " MHz";
        if (frequency == "0.00 MHz") {
            frequency = QObject::tr("This CPU model does not support frequency display function");
        }
        return frequency;
    }else{
        QProcess process;
        process.start("cpufreq-info");
        process.waitForFinished();

        QString output = process.readAllStandardOutput();
        QStringList lines = output.split("\n");

        QStringList currentFrequencies;
        for (const QString &line: lines) {
            if (line.contains("current CPU frequency")) {
                QStringList parts = line.split("is");
                if (parts.size() > 1) {
                    QString currentFrequency = parts.at(1).trimmed().split(" ")[0];
                    if (currentFrequency.contains(".")) {
                        currentFrequency = QString::number(currentFrequency.toFloat() * 1000);
                        currentFrequencies.append(currentFrequency);
                    } else {
                        currentFrequencies.append(currentFrequency);
                    }
                }
            }
        }
        double max = 0;
        for (int i = 0; i < currentFrequencies.length(); ++i) {
            if (max < currentFrequencies[i].toDouble()) {
                max = currentFrequencies[i].toDouble();
            }
        }
        QString frequency = QString::number(max, 'f', 2) + " MHz";
        if (frequency == "0.00 MHz") {
            frequency = QObject::tr("This CPU model does not support frequency display function");
        }
        return frequency;
    }
}

QString CpuInfo::getUsageRate() {
    // CPU 使用率
    QFile file("/proc/stat");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine(); // 读取第一行数据
        QStringList values = line.split(" ", QString::SkipEmptyParts);

        // 读取当前的 CPU 时间数据
        unsigned long long currentWorkTime = 0, currentTotalTime = 0;
        for (int i = 1; i < values.size(); ++i) {
            quint64 value = values[i].toULongLong();
            currentTotalTime += value;
            if (i != 4) { // 第4个值为idle time，其他都是work time
                currentWorkTime += value;
            }
        }

        // 计算 CPU 使用率
        if (m_prevCpuWorkTime != 0 && m_prevCpuTotalTime != 0) {
            double rate = (currentWorkTime - m_prevCpuWorkTime) * 100.0 / (currentTotalTime - m_prevCpuTotalTime);
            m_prevCpuWorkTime = currentWorkTime;
            m_prevCpuTotalTime = currentTotalTime;
            file.close();
            return QString::number(rate, 'f', 2) + "%";
        } else {
            // 更新前一次的 CPU 时间数据
            m_prevCpuWorkTime = currentWorkTime;
            m_prevCpuTotalTime = currentTotalTime;
            file.close();
            return "0.0%"; // 首次读取，没有先前数据可用于计算
        }
    } else {
        return "";
    }
}

QString CpuInfo::getTemprature() {
    // CPU 温度
    int i = 0;
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
    temperature = temperature / i;
    QString temp = QString::number(temperature, 'f', 1) + "°C";
    if (temp != "nan°C"){
        return temp;
    }
    temp = "nan°C";
    // 尝试使用sensors获取一下
    QProcess process;
    process.start("sensors");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n");
    process.close();

    if (temp == "nan°C" && (!output.contains("coretemp-isa")) && (!output.contains("cputemp-isa"))) {
        return QApplication::tr("This CPU model does not support temperature display function");
    }else{
        int coreNum = 0;
        float tempCount = 0;
        for (int j = 0; j < lines.length(); ++j) {
            if (lines[j].contains("Core")) {
                coreNum++;
                temp = QString(lines[j].split(":", QString::SkipEmptyParts)[1]).split(" ", QString::SkipEmptyParts)[0];
                if (temp.contains("+")){
                    temp.remove("+");
                }
                if (temp.contains("°C")){
                    temp.remove("°C");
                }
                tempCount += temp.toFloat();
            }
        }
        float cputemp = tempCount / coreNum;
        return QString::number(cputemp, 'f', 1) + "°C";
    }
}



