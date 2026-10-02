#include "batteryinfo.h"
#include <QString>
#include <QProcess>

BatteryInfo::BatteryInfo() {

}

QString BatteryInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        if (model == "") {
            return "Battery";
        } else {
            return model;
        }
    } else if (value == "serailnum") {
        return serailnum;
    } else if (value == "state") {
        return getState();
    } else if (value == "energy") {
        return getEnergy();
    } else if (value == "used_times") {
        return used_times;
    } else if (value == "energy_full") {
        return energy_full;
    } else if (value == "time_to_empty") {
        return getTimeToEmpty();
    } else if (value == "percentage") {
        return getPercentage();
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

QString BatteryInfo::getEnergy() {
    QProcess process;
    process.start("upower --dump");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');
    for (int i = 0; i < lines.length(); ++i) {
        if (lines[i].contains("energy:")) {
            QString parts = QString(lines[i].split("energy:")[1]).trimmed();
            return parts;
        }
    }
    return energy;
}

QString BatteryInfo::getPercentage() {
    QProcess process;
    process.start("upower --dump");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');
    for (int i = 0; i < lines.length(); ++i) {
        if (lines[i].contains("percentage:")) {
            QString parts = QString(lines[i].split("percentage:")[1]).trimmed();
            return parts;
        }
    }
    return percentage;
}

QString BatteryInfo::getTimeToEmpty() {
    QProcess process;
    process.start("upower --dump");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');
    for (int i = 0; i < lines.length(); ++i) {
        if (lines[i].contains("time to empty:")) {
            QString parts = QString(lines[i].split("time to empty:")[1]).trimmed();
            return parts;
        }
    }
    return time_to_empty;
}

QString BatteryInfo::getState() {
    QProcess process;
    process.start("upower --dump");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');
    for (int i = 0; i < lines.length(); ++i) {
        if (lines[i].contains("state:")) {
            QString parts = QString(lines[i].split("state:")[1]).trimmed();
            return parts;
        }
    }
    return state;
}

void BatteryInfo::setValue(QString key, QString value) {
    if (key == "vendor") {
        vendor = value;
    } else if (key == "model") {
        model = value;
    } else if (key == "serailnum") {
        serailnum = value;
    } else if (key == "state") {
        state = value;
    } else if (key == "energy") {
        energy = value;
    } else if (key == "used_times") {
        used_times = value;
    } else if (key == "energy_full") {
        energy_full = value;
    } else if (key == "time_to_empty") {
        time_to_empty = value;
    } else if (key == "percentage") {
        percentage = value;
    }
}
