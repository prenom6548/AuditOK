#include "monitorinfo.h"
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QProcess>

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
    QMap<QString, QString> devlist;
    QProcess process;
    process.start("xrandr --prop");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');

    QString currentDisplay;
    bool isCurrentMode = false;

    for (const QString &line : lines) {
        if (line.contains(" connected")) {
            // 解析当前显示器名称
            QStringList parts = line.split(' ');
            currentDisplay = parts[0];
            isCurrentMode = false;
        } else if (!currentDisplay.isEmpty() && line.startsWith("   ")) {
            // 解析当前显示器的分辨率和刷新率
            QStringList parts = line.simplified().split(' ');
            if (parts.size() >= 2) {
                QString resolution = parts[0];
                QString refreshRate;
                for(int i = 1; i < parts.length(); ++i){
                    if(QString(parts[i]).contains("*")){
                        refreshRate = parts[i];
                    }
                }
                bool hasAsterisk = refreshRate.contains('*');

                // 如果刷新率带有 '*'，表示当前模式
                if (hasAsterisk) {
                    refreshRate = refreshRate.remove('*');
                    devlist[currentDisplay] = refreshRate;
                    isCurrentMode = true;
                } else if (!isCurrentMode && !devlist.contains(currentDisplay)) {
                    // 如果没有 '*' 并且还没有记录当前显示器的刷新率
                    devlist[currentDisplay] = refreshRate;
                }
            }
        }
    }

    QList<QScreen*> screens = QGuiApplication::screens();
    foreach (QScreen *screen, screens) {
        QString interface = screen->name();
        if (devlist.contains(interface)) {
            QString resolution = QString::number(screen->geometry().width() * screen->devicePixelRatio())
                                + "x"
                                + QString::number(screen->geometry().height() * screen->devicePixelRatio())
                                + "@"
                                + devlist.value(interface) + "Hz";
            return resolution;
        }
    }

    return QString(); // 如果没有匹配到则返回空字符串
}

bool MonitorInfo::isMainMonitor(){
    QScreen* screen = QGuiApplication::primaryScreen();
    if(screen->name() == this->interface){
        return true;
    }else{
        return false;
    }
}

MonitorInfo::~MonitorInfo() {
}


