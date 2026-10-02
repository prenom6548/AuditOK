#include "soundcardinfo.h"
#include <QDebug>
#include <QProcess>

SoundCardInfo::SoundCardInfo() {

}

QString SoundCardInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        if (model == "" && name == "") {
            return "Sound Card";
        } else if (model != "") {
            return model;
        } else if (name != "") {
            return name;
        }
    } else if (value == "serial") {
        return serial;
    } else if (value == "name") {
        return name;
    } else if (value == "deviceId") {
        return deviceId;
    } else if (value == "others") {
        return others;
    } else if (value == "driver_now") {
        return driver_now;
    } else if (value == "driver_version_now") {
        return driver_version_now;
    } else if (value == "sysPath") {
        return sysPath;
    } else if (value == "devicePath") {
        return devicePath;
    } else if (value == "busNumber") {
        return busNumber;
    } else if (value == "deviceNumber") {
        return deviceNumber;
    } else if (value == "VID") {
        return VID;
    } else if (value == "PID") {
        return PID;
    } else if (value == "upgradeable") {
        return QString::number(upgradeable);
    } else if (value == "isOnline") {
        return QString::number(isOnline);
    } else if (value == "bus") {
        return bus;
    } else if (value == "clock") {
        if (clock == "0 MHz") {
            return "";
        } else {
            return clock;
        }
    } else if (value == "width") {
        if (width == "0 bits") {
            return "";
        } else {
            return width;
        }
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

void SoundCardInfo::disableSoundCard() {
    QProcess process;

    // 获取所有音频输出设备的列表
    process.start("pactl list short sinks");
    process.waitForFinished();

    // 读取命令输出
    QString output = process.readAllStandardOutput();
    qDebug() << "Available sinks:\n" << output;

    // 找到要禁用的设备的名称或索引
    // 这里假设我们要禁用的设备是第一个设备
    QStringList lines = output.split('\n');
    if (lines.isEmpty()) {
        qWarning() << "No sinks found!";
        return;
    }

    // 提取第一个设备的名称（假设第一个字段是设备名称）
    QStringList firstDevice = lines.first().split('\t');
    if (firstDevice.isEmpty()) {
        qWarning() << "Failed to parse sink information!";
        return;
    }

    QString sinkName = firstDevice.first();
    qDebug() << "Disabling sink:" << sinkName;

    // 执行命令禁用设备
    QString command = QString("pactl set-sink-mute %1 1").arg(sinkName);
    process.start(command);
    process.waitForFinished();

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        qDebug() << "Sink" << sinkName << "disabled successfully.";
    } else {
        qWarning() << "Failed to disable sink" << sinkName << ":\n" << process.readAllStandardError();
    }
}

void SoundCardInfo::enableSoundCard() {
    QProcess process;

    // 获取所有音频输出设备的列表
    process.start("pactl list short sinks");
    process.waitForFinished();

    // 读取命令输出
    QString output = process.readAllStandardOutput();
    qDebug() << "Available sinks:\n" << output;

    // 找到要启用的设备的名称或索引
    QStringList lines = output.split('\n');
    if (lines.isEmpty()) {
        qWarning() << "No sinks found!";
        return;
    }

    // 提取第一个设备的名称（假设第一个字段是设备名称）
    QStringList firstDevice = lines.first().split('\t');
    if (firstDevice.isEmpty()) {
        qWarning() << "Failed to parse sink information!";
        return;
    }

    QString sinkName = firstDevice.first();
    qDebug() << "Enabling sink:" << sinkName;

    // 执行命令启用设备
    QString command = QString("pactl set-sink-mute %1 0").arg(sinkName);
    process.start(command);
    process.waitForFinished();

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        qDebug() << "Sink" << sinkName << "enabled successfully.";
    } else {
        qWarning() << "Failed to enable sink" << sinkName << ":\n" << process.readAllStandardError();
    }
}

bool SoundCardInfo::checkStatus() {
    QProcess process;
    QStringList env = QProcess::systemEnvironment();
    env << "LANGUAGE=en_us";
    process.setEnvironment(env);
    process.start("pactl list sinks");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    process.close();
    QStringList lines = output.split('\n', QString::SkipEmptyParts);

    QString sinkName;
    bool isMuted = false;
    foreach (const QString &line, lines) {
        if (line.startsWith("Sink #")) {
            // 处理新的音频输出设备
            if (!sinkName.isEmpty()) {
                // 如果之前有状态，输出之前的设备信息
            }
            sinkName = line;  // 获取新的 sink 名称
            isMuted = false;  // 重置静音状态
        }

        if (line.contains("Mute: yes")) {
            isMuted = true;  // 设备被静音
        }
    }
    return isMuted;
}

void SoundCardInfo::setValue(QString key, QString value) {
    if (key == "vendor") {
        vendor = value;
    } else if (key == "model") {
        model = value;
    } else if (key == "serial") {
        serial = value;
    } else if (key == "serial") {
        serial = value;
    } else if (key == "name") {
        name = value;
    } else if (key == "deviceId") {
        deviceId = value;
    } else if (key == "others") {
        others = value;
    } else if (key == "driver_now") {
        driver_now = value;
    } else if (key == "driver_version_now") {
        driver_version_now = value;
    } else if (key == "sysPath") {
        sysPath = value;
    } else if (key == "devicePath") {
        devicePath = value;
    } else if (key == "busNumber") {
        busNumber = value;
    }else if (key == "deviceNumber") {
        deviceNumber = value;
    }else if (key == "VID") {
        VID = value;
    }else if (key == "PID") {
        PID = value;
    }else if (key == "bus") {
        bus = value;
    }else if (key == "clock") {
        clock = value;
    }else if (key == "width") {
        width = value;
    }
}
