#include <QCoreApplication>
#include <QDebug>
#include "../src/kthememanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    kdk::KThemeManager manager;

    QObject::connect(&manager, &kdk::KThemeManager::changed, [=](const QString property){
        qDebug() << property << "changed";
    });

    qDebug() << "UKCC Global Theme: " << manager.getUkccGlobalTheme();

    qDebug() << "Support style: " << manager.getSupportSystemStyle();

    qDebug() << "System font: " << manager.getSystemFont();

    qDebug() << "System font size: " << manager.getSystemFontSize();

    qDebug() << "System color: " << manager.getSystemThemeColor();

    qDebug() << "Set system style: " << manager.setSystemStyle("ukui-default");

    qDebug() << "Set system font: " << manager.setSystemFont("Noto Sans CJK SC");

    qDebug() << "Set system font size: " << manager.setSystemFontSize(12);

    qDebug() << "Set theme color: " << manager.setSystemThemeColor("daybreakBlue");

    qDebug() << "Set panel special effect: " << manager.setPanelSpecialEffect(true);

    qDebug() << "Set transparency: " << manager.setTransparency(0.70);

    return a.exec();
}
