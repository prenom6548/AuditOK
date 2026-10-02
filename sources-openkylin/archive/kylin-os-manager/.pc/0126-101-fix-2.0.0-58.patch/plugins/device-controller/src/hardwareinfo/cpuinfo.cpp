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
        if(currentFrequencies.length() == 0){
            QProcess process2;
            process2.start("cat /tmp/kylin-os-manager-cpu.dat");
            process2.waitForFinished();

            QString output2 = process2.readAllStandardOutput();
            process2.close();
            QStringList lines2 = output2.split("\n");
            for (int i = 0; i < lines2.length(); ++i) {
                if (lines2[i].contains("Current Speed:")){
                    QString freq = lines2[i].split(":")[1].trimmed();
                    return freq;
                }
            }
            return QObject::tr("This CPU model does not support frequency display function");
        }
        double min = currentFrequencies[0].toDouble();
        for (int i = 0; i < currentFrequencies.length(); ++i) {
            if (min > currentFrequencies[i].toDouble()) {
                min = currentFrequencies[i].toDouble();
            }
        }
        QString frequency = QString::number(int(min)) + " MHz";
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

        if(currentFrequencies.length() == 0){
            QProcess process2;
            process2.start("cat /tmp/kylin-os-manager-cpu.dat");
            process2.waitForFinished();

            QString output2 = process2.readAllStandardOutput();
            process2.close();
            QStringList lines2 = output2.split("\n");
            for (int i = 0; i < lines2.length(); ++i) {
                if (lines2[i].contains("Current Speed:")){
                    QString freq = lines2[i].split(":")[1].trimmed();
                    return freq;
                }
            }
            return QObject::tr("This CPU model does not support frequency display function");
        }

        QString frequency = QString::number(int(max)) + " MHz";
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
    QString temp = "nan°C";
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

void CpuInfo::setValue(QString key, QString value) {
    if (key == "vendor") {
        vendor = value;
    } else if (key == "model") {
        model = value;
    } else if (key == "arch") {
        arch = value;
    } else if (key == "cores") {
        cores = value;
    } else if (key == "onlinecores") {
        onlinecores = value;
    } else if (key == "threadprecore") {
        threadprecore = value;
    } else if (key == "maxMHz") {
        maxMHz = value;
    } else if (key == "L1dcache") {
        L1dcache = value;
    } else if (key == "L1icache") {
        L1icache = value;
    } else if (key == "L2cache") {
        L2cache = value;
    } else if (key == "L3cache") {
        L3cache = value;
    } else if (key == "L4cache") {
        L4cache = value;
    }
}



