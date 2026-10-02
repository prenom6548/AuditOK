#include "common.h"

QString getDebianArchitecture() {
    QStringList command = QStringList{"dpkg", "--print-architecture"};
    QProcess proc;
    QStringList options;
    options << "-c" << command.join(" ");
    proc.closeWriteChannel();
    proc.start("bash", options);
    if (!proc.waitForFinished())
        return "";
    QString res = QString(proc.readAll());
    proc.close();
    if (res.right(1) == "\n")
        res.chop(1);
    return res;
}