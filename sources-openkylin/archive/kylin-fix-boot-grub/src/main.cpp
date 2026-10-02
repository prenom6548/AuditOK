#include <QApplication>
#include <QStandardPaths>
#include <QFile>
#include "widget.h"
#include "appLog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    setenv("QT_QPA_PLATFORMTHEME", "ukui", true);
    QString filePath = "/proc/cmdline";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    if (!content.contains("kyrecover")) {
        qDebug("kylin grub Repair tool can only be used live system.");
        return 0;
    }
    // 加入日志打印
    setLogLevel(QtDebugMsg);
    setLogPath("/var/log" + LOG_FILE_NAME);  /* 绑定打印日志文件路径 */
    qInstallMessageHandler(customLogMessageHandler);
    Widget w;
    w.show();
    return a.exec();
}
