#include "graphiccardinfo.h"
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QStringList>
#include <QDebug>

GraphicCardInfo::GraphicCardInfo()
{

}

QString GraphicCardInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        if (model == ""){
            return "Graphic Card Device";
        }
        return model;
    } else if (value == "serial") {
        return serial;
    } else if (value == "name") {
        return name;
    } else if (value == "deviceId") {
        return deviceId;
    } else if (value == "driver_now") {
        return driver_now;
    } else if (value == "driver_version_now") {
        return driver_version_now;
    } else if (value == "modalias") {
        return modalias;
    } else if (value == "subsystem") {
        return subsystem;
    } else if (value == "memorysize") {
        return memorysize;
    } else if (value == "businfo") {
        return businfo;
    } else if (value == "clock") {
        return clock;
    } else if (value == "physid") {
        return physid;
    } else if (value == "version") {
        return version;
    } else if (value == "width") {
        return getGpuBusWidth();
    } else if(value == "gddrsize"){
        return getGDDRSize();
    }else if(value == "egl_version"){
        return egl_version;
    }else if(value == "egl_apis"){
        return egl_apis;
    }else if(value == "gl_version"){
        return gl_version;
    }else if(value == "glsl_version"){
        return glsl_version;
    }else if(value == "gputype"){
        return getGpuType();
    }else{
        return "";
    }
}

QString GraphicCardInfo::getGpuBusWidth() {
    QString logFilePath = "/tmp/kgc.log";
    QFile logFile(logFilePath);

    // 检查文件是否存在
    if (!logFile.exists()) {
        // 运行 kgc-daemon 命令
        QProcess::execute("kgc-daemon");

        // 等待文件生成
        if (!logFile.exists()) {
            qDebug() << "Error: /tmp/kgc.log not created.";
            return width;
        }
    }

    // 打开文件
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: Cannot open /tmp/kgc.log.";
        return "N/A";
    }

    QTextStream in(&logFile);
    QString line;

    // 读取文件中的每一行
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.contains("get_vram_type")) {
            // 找到包含 "get_vram_type" 的行
            QStringList parts = line.split(' ');
            QStringList nonEmptyParts;

            // 过滤掉空部分
            for (const QString &part : parts) {
                if (!part.isEmpty()) {
                    nonEmptyParts.append(part);
                }
            }

            if (nonEmptyParts.size() >= 4) {
                logFile.close();
                if (nonEmptyParts.contains("NULL")){
                    return "N/A";
                }else if(nonEmptyParts[4] == "0"){
                    return QObject::tr("This graphics card does not support obtaining GPU bus bit width");
                }else{
                    return nonEmptyParts[4] + " bits"; // 返回 get_vram_type 后面的内容
                }
            }
        }
    }

    logFile.close();
    qDebug() << "Error: 'get_vram_type' not found in /tmp/kgc.log.";
    return width;
}

QString GraphicCardInfo::getGDDRSize() {
    QProcess process;
    process.start("gpuinfo");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n");
    process.close();
    for (int i = 0; i < lines.length(); ++i) {
        if (lines[i].contains("GDDR capacity")) {
            QString parts = QString(lines[i]).split(":").last();
            return parts.trimmed();
        }
    }
    return "";
}

QString GraphicCardInfo::getGpuType() {
    QProcess process;
    process.start("gpuinfo");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n");
    process.close();
    for (int i = 0; i < lines.length(); ++i) {
        if (lines[i].contains("GPU type")) {
            QString parts = QString(lines[i]).split(":").last();
            return parts.trimmed();
        }
    }
    return "";
}
