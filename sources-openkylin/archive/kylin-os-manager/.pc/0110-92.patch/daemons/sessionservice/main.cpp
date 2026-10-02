#include <QCoreApplication>
#include <QTranslator>
#include <QDebug>
#include "sessionclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QString tranPath("/usr/share/kylin-os-manager/translations");
    QTranslator *translator = new QTranslator;
    if (translator->load(QLocale(), "kylin-os-manager-session-service", "_", tranPath)) {
        QCoreApplication::installTranslator(translator);
    } else {
        qDebug() << "ProblemFeedback load translation file fail !";
    }

    SessionClient sessionClient;
    return app.exec();
}