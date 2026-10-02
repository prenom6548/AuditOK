#include "monitorinfoinitializer.h"
#include <QFile>
#include <QProcess>
#include <QTextStream>

MonitorInfoInitializer::MonitorInfoInitializer() {
    genMonitorInfoDataFile();
}

void MonitorInfoInitializer::genMonitorInfoDataFile() {
    QString strMonitorInfoFile = "/tmp/youker-assistant-monitorinfo.dat";
    QFile tmpFile(strMonitorInfoFile);
    if (tmpFile.exists()) {
        tmpFile.remove();
    }

    QString cmd = "xrandr --prop";
    QProcess p;
    p.start(cmd);
    p.waitForFinished();

    // 将监视器信息写入临时文件
    if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream tsFile(&tmpFile);
    while (p.canReadLine()) {
        QString str = p.readLine();
        str = str.left(str.length() - 1);
        tsFile << str << endl;  // 写入文件
    }
    tmpFile.close();
}
