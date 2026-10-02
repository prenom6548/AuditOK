#include "widget.h"
#include "xatom-helper.h"
#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLocale>
#include "../dialog.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator qtTranslator;
    QString locale = QLocale::system().name();
    if (locale == "zh_CN") {
        qtTranslator.load(":/zh_CN.qm");
        a.installTranslator(&qtTranslator);
    }
    else
    {
        qtTranslator.load(":/en.US.qm");
        a.installTranslator(&qtTranslator);
    }
    QString initWinType(argv[2]);
    Widget w;

    if(argc != 3)
    {
        qDebug() << "Invalid args for miraclecast-frontend";
        return 0;
    }

    qDebug() << "peerName is " <<argv[1]<<", type "<<initWinType;

    w.setConnectDevName(argv[1]);

    if(initWinType == QString("Formationing"))
        w.setInitWinType(INIT_WIDGET_WITHOUT_CONFIRM);
    else
        w.setInitWinType(INIT_WIDGET_WITH_CONFIRM);

    // 添加窗管协议
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(w.winId(), hints);
    
    w.showInitWin();
    return a.exec();
}
