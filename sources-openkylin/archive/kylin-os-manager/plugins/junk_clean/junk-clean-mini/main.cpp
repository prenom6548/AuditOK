#include <QDebug>
#include <QCoreApplication>
#include "junkcleanmini.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    // 参数解析
    QStringList args =  app.arguments();
    args.removeFirst();
#ifdef DEBUG_MODE
    if (args.isEmpty()) {
        args = QStringList()<< "-cleaner=boot";
    }
#endif
    QString cleaner;
    for (QString arg: args) {
        if (!arg.contains("=")) {
            continue;
        }
        if (arg.startsWith("-cleaner")) {
            cleaner = arg.mid(arg.indexOf("=") + 1);
        }
    }
    if (cleaner.isEmpty()){
        qCritical() << "cleaner为空";
        return -1;
    }
    qDebug()<<"解析参数完成，待清理："<<cleaner;

    JunkCleanMini mini(cleaner);
    mini.start();

    return app.exec();
}
