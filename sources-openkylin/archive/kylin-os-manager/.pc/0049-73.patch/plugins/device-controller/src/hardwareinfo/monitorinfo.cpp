#include "monitorinfo.h"
#include <QApplication>
#include <QScreen>

MonitorInfo::MonitorInfo() {

}

QString MonitorInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "name") {
        if(name == ""){
            if (is_main.toInt()){
                return "Main Monitor";
            }else{
                return "Monitor";
            }
        }
        return name;
    } else if (value == "size") {
        return size;
    } else if (value == "ratio") {
        return ratio;
    } else if (value == "resolution") {
        return getResolution();
    } else if (value == "max_resolution") {
        return max_resolution;
    } else if (value == "is_main") {
        if(isMainMonitor()){
            return QApplication::tr("Yes");
        }else{
            return QApplication::tr("No");
        }
    } else if (value == "gamma") {
        return gamma;
    } else if (value == "interface") {
        return interface;
    } else if (value == "model") {
        return model;
    } else if (value == "visible_area") {
        return visible_area;
    } else if (value == "year") {
        return year;
    } else if (value == "week") {
        return week;
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

QString MonitorInfo::getResolution() {
    QList<QScreen*> screens = QGuiApplication::screens();
    foreach (QScreen *screen, screens) {
        if(screen->name() == interface){
            resolution = QString::number(screen->geometry().width() * screen->devicePixelRatio()) + "x" + QString::number(screen->geometry().height() * screen->devicePixelRatio()) + "@" + QString::number(screen->refreshRate()) + ".00Hz";
            break;
        }
    }
    return resolution;
}

bool MonitorInfo::isMainMonitor(){
    QScreen* screen = QGuiApplication::primaryScreen();
    if(screen->name() == interface){
        return true;
    }else{
        return false;
    }
}


