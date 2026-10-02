#include "startuptools.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <stdlib.h>
#include "tools.h"

const QString CONFIGPATH = "/usr/share/kylin-asrassistant/config/";
const QString KEYWORD = "keyword";
const QString VALUEWORD = "path";
const QString BREAKSEP = ";";

QMap<QString, QString> StartUpTools::s_nomalMap = StartUpTools::initConfigNomalMap();
QMap<QString, QString> StartUpTools::s_tabletMap = StartUpTools::initConfigTabletMap();

StartUpTools *StartUpTools::getInstance()
{
    static StartUpTools instance;
    return &instance;
}

QMap<QString, QString> StartUpTools::initConfigNomalMap()
{
    QMap<QString, QString> map;
    // for supported image types
    map.insert("打开多端协同", "/usr/bin/kylin-connectivity");
    map.insert("启动多端协同", "/usr/bin/kylin-connectivity");
    map.insert("唤醒多端协同", "/usr/bin/kylin-connectivity");
    map.insert("开启多端协同", "/usr/bin/kylin-connectivity");
    map.insert("打开多端", "/usr/bin/kylin-connectivity");
    map.insert("启动多端", "/usr/bin/kylin-connectivity");
    map.insert("唤醒多端", "/usr/bin/kylin-connectivity");
    map.insert("开启多端", "/usr/bin/kylin-connectivity");

    map.insert("打开应用商店", "/usr/bin/kylin-software-center");
    map.insert("启动应用商店", "/usr/bin/kylin-software-center");
    map.insert("唤醒应用商店", "/usr/bin/kylin-software-center");
    map.insert("开启应用商店", "/usr/bin/kylin-software-center");
    map.insert("打开软件商店", "/usr/bin/kylin-software-center");
    map.insert("启动软件商店", "/usr/bin/kylin-software-center");
    map.insert("唤醒软件商店", "/usr/bin/kylin-software-center");
    map.insert("开启软件商店", "/usr/bin/kylin-software-center");
    map.insert("打开商店", "/usr/bin/kylin-software-center");
    map.insert("启动商店", "/usr/bin/kylin-software-center");
    map.insert("唤醒商店", "/usr/bin/kylin-software-center");
    map.insert("开启商店", "/usr/bin/kylin-software-center");

    map.insert("打开刻录", "/usr/bin/kylin-burner");
    map.insert("启动刻录", "/usr/bin/kylin-burner");
    map.insert("唤醒刻录", "/usr/bin/kylin-burner");
    map.insert("开启刻录", "/usr/bin/kylin-burner");

    map.insert("打开截图", "/usr/bin/kylin-screenshot gui -d 400");
    map.insert("启动截图", "/usr/bin/kylin-screenshot gui -d 400");
    map.insert("唤醒截图", "/usr/bin/kylin-screenshot gui -d 400");
    map.insert("开启截图", "/usr/bin/kylin-screenshot gui -d 400");

    map.insert("打开字体管理器", "/usr/bin/kylin-font-viewer");
    map.insert("启动字体管理器", "/usr/bin/kylin-font-viewer");
    map.insert("唤醒字体管理器", "/usr/bin/kylin-font-viewer");
    map.insert("开启字体管理器", "/usr/bin/kylin-font-viewer");

    map.insert("打开安装器", "/usr/bin/kylin-installer");
    map.insert("启动安装器", "/usr/bin/kylin-installer");
    map.insert("唤醒安装器", "/usr/bin/kylin-installer");
    map.insert("开启安装器", "/usr/bin/kylin-installer");
    map.insert("打开麒麟安装器", "/usr/bin/kylin-installer");
    map.insert("启动麒麟安装器", "/usr/bin/kylin-installer");
    map.insert("唤醒麒麟安装器", "/usr/bin/kylin-installer");
    map.insert("开启麒麟安装器", "/usr/bin/kylin-installer");

    map.insert("打开KMREAPK安装器", "/usr/bin/kylin-apk-installer");
    map.insert("启动KMREAPK安装器", "/usr/bin/kylin-apk-installer");
    map.insert("唤醒KMREAPK安装器", "/usr/bin/kylin-apk-installer");
    map.insert("开启KMREAPK安装器", "/usr/bin/kylin-apk-installer");
    map.insert("打开APK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("启动APK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("唤醒APK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("开启APK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("打开安卓安装器", "/usr/bin/kmre-apk-installer");
    map.insert("启动安卓安装器", "/usr/bin/kmre-apk-installer");
    map.insert("唤醒安卓安装器", "/usr/bin/kmre-apk-installer");
    map.insert("开启安卓安装器", "/usr/bin/kmre-apk-installer");

    map.insert("打开便签", "/usr/bin/ukui-notebook");
    map.insert("启动便签", "/usr/bin/ukui-notebook");
    map.insert("唤醒便签", "/usr/bin/ukui-notebook");
    map.insert("开启便签", "/usr/bin/ukui-notebook");
    map.insert("打开便签本", "/usr/bin/ukui-notebook");
    map.insert("启动便签本", "/usr/bin/ukui-notebook");
    map.insert("唤醒便签本", "/usr/bin/ukui-notebook");
    map.insert("开启便签本", "/usr/bin/ukui-notebook");
    map.insert("打开记事本", "/usr/bin/ukui-notebook");
    map.insert("启动记事本", "/usr/bin/ukui-notebook");
    map.insert("唤醒记事本", "/usr/bin/ukui-notebook");
    map.insert("开启记事本", "/usr/bin/ukui-notebook");

    map.insert("打开传书", "/usr/bin/kylin-ipmsg");
    map.insert("启动传书", "/usr/bin/kylin-ipmsg");
    map.insert("唤醒传书", "/usr/bin/kylin-ipmsg");
    map.insert("开启传书", "/usr/bin/kylin-ipmsg");
    map.insert("打开通讯工具", "/usr/bin/kylin-ipmsg");
    map.insert("启动通讯工具", "/usr/bin/kylin-ipmsg");
    map.insert("唤醒通讯工具", "/usr/bin/kylin-ipmsg");
    map.insert("开启通讯工具", "/usr/bin/kylin-ipmsg");

    map.insert("打开打印机", "/usr/bin/kylin-printer");
    map.insert("启动打印机", "/usr/bin/kylin-printer");
    map.insert("唤醒打印机", "/usr/bin/kylin-printer");
    map.insert("开启打印机", "/usr/bin/kylin-printer");
    map.insert("打开打印", "/usr/bin/kylin-printer");
    map.insert("启动打印", "/usr/bin/kylin-printer");
    map.insert("唤醒打印", "/usr/bin/kylin-printer");
    map.insert("开启打印", "/usr/bin/kylin-printer");

    map.insert("打开工具箱", "/usr/bin/kylin-assistant");
    map.insert("启动工具箱", "/usr/bin/kylin-assistant");
    map.insert("唤醒工具箱", "/usr/bin/kylin-assistant");
    map.insert("开启工具箱", "/usr/bin/kylin-assistant");

    map.insert("打开归档管理器", "/usr/bin/engrampa");
    map.insert("启动归档管理器", "/usr/bin/engrampa");
    map.insert("唤醒归档管理器", "/usr/bin/engrampa");
    map.insert("开启归档管理器", "/usr/bin/engrampa");
    map.insert("打开归档", "/usr/bin/engrampa");
    map.insert("启动归档", "/usr/bin/engrampa");
    map.insert("唤醒归档", "/usr/bin/engrampa");
    map.insert("开启归档", "/usr/bin/engrampa");

    map.insert("打开计算器", "/usr/bin/kylin-calculator");
    map.insert("启动计算器", "/usr/bin/kylin-calculator");
    map.insert("唤醒计算器", "/usr/bin/kylin-calculator");
    map.insert("开启计算器", "/usr/bin/kylin-calculator");

    map.insert("打开键盘布局查看器", "/usr/bin/kbd-layout-viewer5");
    map.insert("启动键盘布局查看器", "/usr/bin/kbd-layout-viewer5");
    map.insert("唤醒键盘布局查看器", "/usr/bin/kbd-layout-viewer5");
    map.insert("开启键盘布局查看器", "/usr/bin/kbd-layout-viewer5");

    map.insert("打开看图", "/usr/bin/kylin-photo-viewer");
    map.insert("启动看图", "/usr/bin/kylin-photo-viewer");
    map.insert("唤醒看图", "/usr/bin/kylin-photo-viewer");
    map.insert("开启看图", "/usr/bin/kylin-photo-viewer");

    map.insert("打开录音", "/usr/bin/kylin-recorder");
    map.insert("启动录音", "/usr/bin/kylin-recorder");
    map.insert("唤醒录音", "/usr/bin/kylin-recorder");
    map.insert("开启录音", "/usr/bin/kylin-recorder");

    map.insert("打开闹钟", "/usr/bin/ukui-clock");
    map.insert("启动闹钟", "/usr/bin/ukui-clock");
    map.insert("唤醒闹钟", "/usr/bin/ukui-clock");
    map.insert("开启闹钟", "/usr/bin/ukui-clock");

    map.insert("打开麒麟管家", "/usr/bin/kylin-os-manager");
    map.insert("启动麒麟管家", "/usr/bin/kylin-os-manager");
    map.insert("唤醒麒麟管家", "/usr/bin/kylin-os-manager");
    map.insert("开启麒麟管家", "/usr/bin/kylin-os-manager");
    map.insert("打开管家", "/usr/bin/kylin-os-manager");
    map.insert("启动管家", "/usr/bin/kylin-os-manager");
    map.insert("唤醒管家", "/usr/bin/kylin-os-manager");
    map.insert("开启管家", "/usr/bin/kylin-os-manager");

    map.insert("打开扫描", "/usr/bin/kylin-scanner");
    map.insert("启动扫描", "/usr/bin/kylin-scanner");
    map.insert("唤醒扫描", "/usr/bin/kylin-scanner");
    map.insert("开启扫描", "/usr/bin/kylin-scanner");

    map.insert("打开生物特征管理", "/usr/bin/biometric-manager");
    map.insert("启动生物特征管理", "/usr/bin/biometric-manager");
    map.insert("唤醒生物特征管理", "/usr/bin/biometric-manager");
    map.insert("开启生物特征管理", "/usr/bin/biometric-manager");

    map.insert("打开天气", "/usr/bin/kylin-weather showmainwindow");
    map.insert("启动天气", "/usr/bin/kylin-weather showmainwindow");
    map.insert("唤醒天气", "/usr/bin/kylin-weather showmainwindow");
    map.insert("开启天气", "/usr/bin/kylin-weather showmainwindow");

    map.insert("打开文本编辑器", "/usr/bin/pluma");
    map.insert("启动文本编辑器", "/usr/bin/pluma");
    map.insert("唤醒文本编辑器", "/usr/bin/pluma");
    map.insert("开启文本编辑器", "/usr/bin/pluma");
    map.insert("打开文本", "/usr/bin/pluma");
    map.insert("启动文本", "/usr/bin/pluma");
    map.insert("唤醒文本", "/usr/bin/pluma");
    map.insert("开启文本", "/usr/bin/pluma");

    map.insert("打开音乐", "/usr/bin/kylin-music");
    map.insert("启动音乐", "/usr/bin/kylin-music");
    map.insert("唤醒音乐", "/usr/bin/kylin-music");
    map.insert("开启音乐", "/usr/bin/kylin-music");

    map.insert("打开影音", "/usr/bin/kylin-video");
    map.insert("启动影音", "/usr/bin/kylin-video");
    map.insert("唤醒影音", "/usr/bin/kylin-video");
    map.insert("开启影音", "/usr/bin/kylin-video");

    map.insert("打开终端", "/usr/bin/mate-terminal");
    map.insert("启动终端", "/usr/bin/mate-terminal");
    map.insert("唤醒终端", "/usr/bin/mate-terminal");
    map.insert("开启终端", "/usr/bin/mate-terminal");

    map.insert("打开U盘启动器", "/usr/bin/kylin-usb-creator");
    map.insert("启动U盘启动器", "/usr/bin/kylin-usb-creator");
    map.insert("唤醒U盘启动器", "/usr/bin/kylin-usb-creator");
    map.insert("开启U盘启动器", "/usr/bin/kylin-usb-creator");

    map.insert("打开系统监视器", "/usr/bin/ukui-system-monitor");
    map.insert("启动系统监视器", "/usr/bin/ukui-system-monitor");
    map.insert("唤醒系统监视器", "/usr/bin/ukui-system-monitor");
    map.insert("开启系统监视器", "/usr/bin/ukui-system-monitor");

    map.insert("打开用户手册", "/usr/bin/kylin-user-guide -A");
    map.insert("启动用户手册", "/usr/bin/kylin-user-guide -A");
    map.insert("唤醒用户手册", "/usr/bin/kylin-user-guide -A");
    map.insert("开启用户手册", "/usr/bin/kylin-user-guide -A");
    map.insert("打开手册", "/usr/bin/user-guide -A");
    map.insert("启动手册", "/usr/bin/user-guide -A");
    map.insert("唤醒手册", "/usr/bin/user-guide -A");
    map.insert("开启手册", "/usr/bin/user-guide -A");

    map.insert("打开文件管理器", "/usr/bin/peony");
    map.insert("启动文件管理器", "/usr/bin/peony");
    map.insert("唤醒文件管理器", "/usr/bin/peony");
    map.insert("开启文件管理器", "/usr/bin/peony");
    map.insert("打开文管", "/usr/bin/peony");
    map.insert("启动文管", "/usr/bin/peony");
    map.insert("唤醒文管", "/usr/bin/peony");
    map.insert("开启文管", "/usr/bin/peony");

    map.insert("打开开始菜单", "/usr/bin/ukui-menu -s");
    map.insert("启动开始菜单", "/usr/bin/ukui-menu -s");
    map.insert("唤醒开始菜单", "/usr/bin/ukui-menu -s");
    map.insert("开启开始菜单", "/usr/bin/ukui-menu -s");
    map.insert("打开菜单", "/usr/bin/ukui-menu -s");
    map.insert("启动菜单", "/usr/bin/ukui-menu -s");
    map.insert("唤醒菜单", "/usr/bin/ukui-menu -s");
    map.insert("开启菜单", "/usr/bin/ukui-menu -s");

    map.insert("打开侧边栏", "/usr/bin/ukui-sidebar -s control");
    map.insert("启动侧边栏", "/usr/bin/ukui-sidebar -s control");
    map.insert("唤醒侧边栏", "/usr/bin/ukui-sidebar -s control");
    map.insert("开启侧边栏", "/usr/bin/ukui-sidebar -s control");

    map.insert("打开设置", "/usr/bin/ukui-control-center");
    map.insert("启动设置", "/usr/bin/ukui-control-center");
    map.insert("唤醒设置", "/usr/bin/ukui-control-center");
    map.insert("开启设置", "/usr/bin/ukui-control-center");
    map.insert("打开系统设置", "/usr/bin/ukui-control-center");
    map.insert("启动系统设置", "/usr/bin/ukui-control-center");
    map.insert("唤醒系统设置", "/usr/bin/ukui-control-center");
    map.insert("开启系统设置", "/usr/bin/ukui-control-center");
    map.insert("打开控制面板", "/usr/bin/ukui-control-center");
    map.insert("启动控制面板", "/usr/bin/ukui-control-center");
    map.insert("唤醒控制面板", "/usr/bin/ukui-control-center");
    map.insert("开启控制面板", "/usr/bin/ukui-control-center");

    map.insert("打开火狐浏览器", "/usr/bin/firefox");
    map.insert("启动火狐浏览器", "/usr/bin/firefox");
    map.insert("唤醒火狐浏览器", "/usr/bin/firefox");
    map.insert("开启火狐浏览器", "/usr/bin/firefox");
    map.insert("打开浏览器", "/usr/bin/firefox");
    map.insert("启动浏览器", "/usr/bin/firefox");
    map.insert("唤醒浏览器", "/usr/bin/firefox");
    map.insert("开启浏览器", "/usr/bin/firefox");

    map.insert("打开文档", "/usr/bin/wps");
    map.insert("启动文档", "/usr/bin/wps");
    map.insert("唤醒文档", "/usr/bin/wps");
    map.insert("开启文档", "/usr/bin/wps");

    map.insert("打开表格", "/usr/bin/et");
    map.insert("启动表格", "/usr/bin/et");
    map.insert("唤醒表格", "/usr/bin/et");
    map.insert("开启表格", "/usr/bin/et");

    map.insert("打开PPT", "/usr/bin/wpp");
    map.insert("启动PPT", "/usr/bin/wpp");
    map.insert("唤醒PPT", "/usr/bin/wpp");
    map.insert("开启PPT", "/usr/bin/wpp");

    map.insert("打开演示文档", "/usr/bin/wpp");
    map.insert("启动演示文档", "/usr/bin/wpp");
    map.insert("唤醒演示文档", "/usr/bin/wpp");
    map.insert("开启演示文档", "/usr/bin/wpp");

    map.insert("打开PDF", "/usr/bin/wpspdf");
    map.insert("启动PDF", "/usr/bin/wpspdf");
    map.insert("唤醒PDF", "/usr/bin/wpspdf");
    map.insert("开启PDF", "/usr/bin/wpspdf");

    map.insert("打开备份还原", "/usr/bin/kybackup");
    map.insert("启动备份还原", "/usr/bin/kybackup");
    map.insert("唤醒备份还原", "/usr/bin/kybackup");
    map.insert("开启备份还原", "/usr/bin/kybackup");

    return map;
}

QMap<QString, QString> StartUpTools::initConfigTabletMap()
{
    QMap<QString, QString> map;
    // for supported image types
    map.insert("打开多端协同", "/usr/bin/kylin-connectivity");
    map.insert("启动多端协同", "/usr/bin/kylin-connectivity");
    map.insert("唤醒多端协同", "/usr/bin/kylin-connectivity");
    map.insert("开启多端协同", "/usr/bin/kylin-connectivity");
    map.insert("打开多端", "/usr/bin/kylin-connectivity");
    map.insert("启动多端", "/usr/bin/kylin-connectivity");
    map.insert("唤醒多端", "/usr/bin/kylin-connectivity");
    map.insert("开启多端", "/usr/bin/kylin-connectivity");

    map.insert("打开应用商店", "/usr/bin/kylin-software-center");
    map.insert("启动应用商店", "/usr/bin/kylin-software-center");
    map.insert("唤醒应用商店", "/usr/bin/kylin-software-center");
    map.insert("开启应用商店", "/usr/bin/kylin-software-center");
    map.insert("打开软件商店", "/usr/bin/kylin-software-center");
    map.insert("启动软件商店", "/usr/bin/kylin-software-center");
    map.insert("唤醒软件商店", "/usr/bin/kylin-software-center");
    map.insert("开启软件商店", "/usr/bin/kylin-software-center");
    map.insert("打开商店", "/usr/bin/kylin-software-center");
    map.insert("启动商店", "/usr/bin/kylin-software-center");
    map.insert("唤醒商店", "/usr/bin/kylin-software-center");
    map.insert("开启商店", "/usr/bin/kylin-software-center");

    map.insert("打开刻录", "/usr/bin/kylin-burner");
    map.insert("启动刻录", "/usr/bin/kylin-burner");
    map.insert("唤醒刻录", "/usr/bin/kylin-burner");
    map.insert("开启刻录", "/usr/bin/kylin-burner");

    map.insert("打开截图", "/usr/bin/kylin-screenshot gui -d 400");
    map.insert("启动截图", "/usr/bin/kylin-screenshot gui -d 400");
    map.insert("唤醒截图", "/usr/bin/kylin-screenshot gui -d 400");
    map.insert("开启截图", "/usr/bin/kylin-screenshot gui -d 400");

    map.insert("打开字体管理器", "/usr/bin/kylin-font-viewer");
    map.insert("启动字体管理器", "/usr/bin/kylin-font-viewer");
    map.insert("唤醒字体管理器", "/usr/bin/kylin-font-viewer");
    map.insert("开启字体管理器", "/usr/bin/kylin-font-viewer");

    map.insert("打开安装器", "/usr/bin/kylin-installer");
    map.insert("启动安装器", "/usr/bin/kylin-installer");
    map.insert("唤醒安装器", "/usr/bin/kylin-installer");
    map.insert("开启安装器", "/usr/bin/kylin-installer");
    map.insert("打开麒麟安装器", "/usr/bin/kylin-installer");
    map.insert("启动麒麟安装器", "/usr/bin/kylin-installer");
    map.insert("唤醒麒麟安装器", "/usr/bin/kylin-installer");
    map.insert("开启麒麟安装器", "/usr/bin/kylin-installer");

    map.insert("打开KMREAPK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("启动KMREAPK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("唤醒KMREAPK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("开启KMREAPK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("打开APK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("启动APK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("唤醒APK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("开启APK安装器", "/usr/bin/kmre-apk-installer");
    map.insert("打开安卓安装器", "/usr/bin/kmre-apk-installer");
    map.insert("启动安卓安装器", "/usr/bin/kmre-apk-installer");
    map.insert("唤醒安卓安装器", "/usr/bin/kmre-apk-installer");
    map.insert("开启安卓安装器", "/usr/bin/kmre-apk-installer");

    map.insert("打开便签", "/usr/bin/ukui-notebook");
    map.insert("启动便签", "/usr/bin/ukui-notebook");
    map.insert("唤醒便签", "/usr/bin/ukui-notebook");
    map.insert("开启便签", "/usr/bin/ukui-notebook");
    map.insert("打开便签本", "/usr/bin/ukui-notebook");
    map.insert("启动便签本", "/usr/bin/ukui-notebook");
    map.insert("唤醒便签本", "/usr/bin/ukui-notebook");
    map.insert("开启便签本", "/usr/bin/ukui-notebook");
    map.insert("打开记事本", "/usr/bin/ukui-notebook");
    map.insert("启动记事本", "/usr/bin/ukui-notebook");
    map.insert("唤醒记事本", "/usr/bin/ukui-notebook");
    map.insert("开启记事本", "/usr/bin/ukui-notebook");

    map.insert("打开传书", "/usr/bin/kylin-ipmsg");
    map.insert("启动传书", "/usr/bin/kylin-ipmsg");
    map.insert("唤醒传书", "/usr/bin/kylin-ipmsg");
    map.insert("开启传书", "/usr/bin/kylin-ipmsg");
    map.insert("打开通讯工具", "/usr/bin/kylin-ipmsg");
    map.insert("启动通讯工具", "/usr/bin/kylin-ipmsg");
    map.insert("唤醒通讯工具", "/usr/bin/kylin-ipmsg");
    map.insert("开启通讯工具", "/usr/bin/kylin-ipmsg");

    map.insert("打开打印机", "/usr/bin/kylin-printer");
    map.insert("启动打印机", "/usr/bin/kylin-printer");
    map.insert("唤醒打印机", "/usr/bin/kylin-printer");
    map.insert("开启打印机", "/usr/bin/kylin-printer");
    map.insert("打开打印", "/usr/bin/kylin-printer");
    map.insert("启动打印", "/usr/bin/kylin-printer");
    map.insert("唤醒打印", "/usr/bin/kylin-printer");
    map.insert("开启打印", "/usr/bin/kylin-printer");

    map.insert("打开工具箱", "/usr/bin/kylin-assistant");
    map.insert("启动工具箱", "/usr/bin/kylin-assistant");
    map.insert("唤醒工具箱", "/usr/bin/kylin-assistant");
    map.insert("开启工具箱", "/usr/bin/kylin-assistant");

    map.insert("打开归档管理器", "/usr/bin/engrampa");
    map.insert("启动归档管理器", "/usr/bin/engrampa");
    map.insert("唤醒归档管理器", "/usr/bin/engrampa");
    map.insert("开启归档管理器", "/usr/bin/engrampa");
    map.insert("打开归档", "/usr/bin/engrampa");
    map.insert("启动归档", "/usr/bin/engrampa");
    map.insert("唤醒归档", "/usr/bin/engrampa");
    map.insert("开启归档", "/usr/bin/engrampa");

    map.insert("打开计算器", "/usr/bin/kylin-calculator");
    map.insert("启动计算器", "/usr/bin/kylin-calculator");
    map.insert("唤醒计算器", "/usr/bin/kylin-calculator");
    map.insert("开启计算器", "/usr/bin/kylin-calculator");

    map.insert("打开键盘布局查看器", "/usr/bin/kbd-layout-viewer5");
    map.insert("启动键盘布局查看器", "/usr/bin/kbd-layout-viewer5");
    map.insert("唤醒键盘布局查看器", "/usr/bin/kbd-layout-viewer5");
    map.insert("开启键盘布局查看器", "/usr/bin/kbd-layout-viewer5");

    map.insert("打开看图", "/usr/bin/kylin-photo-viewer");
    map.insert("启动看图", "/usr/bin/kylin-photo-viewer");
    map.insert("唤醒看图", "/usr/bin/kylin-photo-viewer");
    map.insert("开启看图", "/usr/bin/kylin-photo-viewer");

    map.insert("打开录音", "/usr/bin/kylin-recorder");
    map.insert("启动录音", "/usr/bin/kylin-recorder");
    map.insert("唤醒录音", "/usr/bin/kylin-recorder");
    map.insert("开启录音", "/usr/bin/kylin-recorder");

    map.insert("打开闹钟", "/usr/bin/ukui-clock");
    map.insert("启动闹钟", "/usr/bin/ukui-clock");
    map.insert("唤醒闹钟", "/usr/bin/ukui-clock");
    map.insert("开启闹钟", "/usr/bin/ukui-clock");

    map.insert("打开麒麟管家", "/usr/bin/kylin-os-manager");
    map.insert("启动麒麟管家", "/usr/bin/kylin-os-manager");
    map.insert("唤醒麒麟管家", "/usr/bin/kylin-os-manager");
    map.insert("开启麒麟管家", "/usr/bin/kylin-os-manager");
    map.insert("打开管家", "/usr/bin/kylin-os-manager");
    map.insert("启动管家", "/usr/bin/kylin-os-manager");
    map.insert("唤醒管家", "/usr/bin/kylin-os-manager");
    map.insert("开启管家", "/usr/bin/kylin-os-manager");

    map.insert("打开扫描", "/usr/bin/kylin-scanner");
    map.insert("启动扫描", "/usr/bin/kylin-scanner");
    map.insert("唤醒扫描", "/usr/bin/kylin-scanner");
    map.insert("开启扫描", "/usr/bin/kylin-scanner");

    map.insert("打开生物特征管理", "/usr/bin/biometric-manager");
    map.insert("启动生物特征管理", "/usr/bin/biometric-manager");
    map.insert("唤醒生物特征管理", "/usr/bin/biometric-manager");
    map.insert("开启生物特征管理", "/usr/bin/biometric-manager");

    map.insert("打开天气", "/usr/bin/kylin-weather showmainwindow");
    map.insert("启动天气", "/usr/bin/kylin-weather showmainwindow");
    map.insert("唤醒天气", "/usr/bin/kylin-weather showmainwindow");
    map.insert("开启天气", "/usr/bin/kylin-weather showmainwindow");

    map.insert("打开文本编辑器", "/usr/bin/pluma");
    map.insert("启动文本编辑器", "/usr/bin/pluma");
    map.insert("唤醒文本编辑器", "/usr/bin/pluma");
    map.insert("开启文本编辑器", "/usr/bin/pluma");
    map.insert("打开文本", "/usr/bin/pluma");
    map.insert("启动文本", "/usr/bin/pluma");
    map.insert("唤醒文本", "/usr/bin/pluma");
    map.insert("开启文本", "/usr/bin/pluma");

    map.insert("打开音乐", "/usr/bin/kylin-music");
    map.insert("启动音乐", "/usr/bin/kylin-music");
    map.insert("唤醒音乐", "/usr/bin/kylin-music");
    map.insert("开启音乐", "/usr/bin/kylin-music");

    map.insert("打开影音", "/usr/bin/kylin-video");
    map.insert("启动影音", "/usr/bin/kylin-video");
    map.insert("唤醒影音", "/usr/bin/kylin-video");
    map.insert("开启影音", "/usr/bin/kylin-video");

    map.insert("打开终端", "/usr/bin/mate-terminal");
    map.insert("启动终端", "/usr/bin/mate-terminal");
    map.insert("唤醒终端", "/usr/bin/mate-terminal");
    map.insert("开启终端", "/usr/bin/mate-terminal");

    map.insert("打开U盘启动器", "/usr/bin/kylin-usb-creator");
    map.insert("启动U盘启动器", "/usr/bin/kylin-usb-creator");
    map.insert("唤醒U盘启动器", "/usr/bin/kylin-usb-creator");
    map.insert("开启U盘启动器", "/usr/bin/kylin-usb-creator");

    map.insert("打开系统监视器", "/usr/bin/ukui-system-monitor");
    map.insert("启动系统监视器", "/usr/bin/ukui-system-monitor");
    map.insert("唤醒系统监视器", "/usr/bin/ukui-system-monitor");
    map.insert("开启系统监视器", "/usr/bin/ukui-system-monitor");

    map.insert("打开用户手册", "/usr/bin/kylin-user-guide -A");
    map.insert("启动用户手册", "/usr/bin/kylin-user-guide -A");
    map.insert("唤醒用户手册", "/usr/bin/kylin-user-guide -A");
    map.insert("开启用户手册", "/usr/bin/kylin-user-guide -A");
    map.insert("打开手册", "/usr/bin/kylin-user-guide -A");
    map.insert("启动手册", "/usr/bin/kylin-user-guide -A");
    map.insert("唤醒手册", "/usr/bin/kylin-user-guide -A");
    map.insert("开启手册", "/usr/bin/kylin-user-guide -A");

    map.insert("打开文件管理器", "/usr/bin/peony");
    map.insert("启动文件管理器", "/usr/bin/peony");
    map.insert("唤醒文件管理器", "/usr/bin/peony");
    map.insert("开启文件管理器", "/usr/bin/peony");
    map.insert("打开文管", "/usr/bin/peony");
    map.insert("启动文管", "/usr/bin/peony");
    map.insert("唤醒文管", "/usr/bin/peony");
    map.insert("开启文管", "/usr/bin/peony");

    map.insert("打开侧边栏", "/usr/bin/ukui-sidebar -s control");
    map.insert("启动侧边栏", "/usr/bin/ukui-sidebar -s control");
    map.insert("唤醒侧边栏", "/usr/bin/ukui-sidebar -s control");
    map.insert("开启侧边栏", "/usr/bin/ukui-sidebar -s control");
    map.insert("打开通知", "/usr/bin/ukui-sidebar -s notify");
    map.insert("启动通知", "/usr/bin/ukui-sidebar -s notify");
    map.insert("唤醒通知", "/usr/bin/ukui-sidebar -s notify");
    map.insert("开启通知", "/usr/bin/ukui-sidebar -s notify");
    map.insert("打开通知中心", "/usr/bin/ukui-sidebar -s notify");
    map.insert("启动通知中心", "/usr/bin/ukui-sidebar -s notify");
    map.insert("唤醒通知中心", "/usr/bin/ukui-sidebar -s notify");
    map.insert("开启通知中心", "/usr/bin/ukui-sidebar -s notify");

    map.insert("打开设置", "/usr/bin/ukui-control-center");
    map.insert("启动设置", "/usr/bin/ukui-control-center");
    map.insert("唤醒设置", "/usr/bin/ukui-control-center");
    map.insert("开启设置", "/usr/bin/ukui-control-center");
    map.insert("打开系统设置", "/usr/bin/ukui-control-center");
    map.insert("启动系统设置", "/usr/bin/ukui-control-center");
    map.insert("唤醒系统设置", "/usr/bin/ukui-control-center");
    map.insert("开启系统设置", "/usr/bin/ukui-control-center");
    map.insert("打开控制面板", "/usr/bin/ukui-control-center");
    map.insert("启动控制面板", "/usr/bin/ukui-control-center");
    map.insert("唤醒控制面板", "/usr/bin/ukui-control-center");
    map.insert("开启控制面板", "/usr/bin/ukui-control-center");

    map.insert("打开火狐浏览器", "/usr/bin/firefox");
    map.insert("启动火狐浏览器", "/usr/bin/firefox");
    map.insert("唤醒火狐浏览器", "/usr/bin/firefox");
    map.insert("开启火狐浏览器", "/usr/bin/firefox");
    map.insert("打开浏览器", "/usr/bin/firefox");
    map.insert("启动浏览器", "/usr/bin/firefox");
    map.insert("唤醒浏览器", "/usr/bin/firefox");
    map.insert("开启浏览器", "/usr/bin/firefox");

    map.insert("打开文档", "/usr/bin/wps");
    map.insert("启动文档", "/usr/bin/wps");
    map.insert("唤醒文档", "/usr/bin/wps");
    map.insert("开启文档", "/usr/bin/wps");

    map.insert("打开表格", "/usr/bin/et");
    map.insert("启动表格", "/usr/bin/et");
    map.insert("唤醒表格", "/usr/bin/et");
    map.insert("开启表格", "/usr/bin/et");

    map.insert("打开PPT", "/usr/bin/wpp");
    map.insert("启动PPT", "/usr/bin/wpp");
    map.insert("唤醒PPT", "/usr/bin/wpp");
    map.insert("开启PPT", "/usr/bin/wpp");

    map.insert("打开演示文档", "/usr/bin/wpp");
    map.insert("启动演示文档", "/usr/bin/wpp");
    map.insert("唤醒演示文档", "/usr/bin/wpp");
    map.insert("开启演示文档", "/usr/bin/wpp");

    map.insert("打开PDF", "/usr/bin/wpspdf");
    map.insert("启动PDF", "/usr/bin/wpspdf");
    map.insert("唤醒PDF", "/usr/bin/wpspdf");
    map.insert("开启PDF", "/usr/bin/wpspdf");

    map.insert("打开备份还原", "/usr/bin/kybackup");
    map.insert("启动备份还原", "/usr/bin/kybackup");
    map.insert("唤醒备份还原", "/usr/bin/kybackup");
    map.insert("开启备份还原", "/usr/bin/kybackup");

    return map;
}

void StartUpTools::loadConfig()
{
    QDir configDir(CONFIGPATH);
    if (!configDir.exists()) {
        qCritical() << "Profile path error!";
        return;
    }
    QFileInfoList fileInfoList = configDir.entryInfoList(QDir::Files);
    for (auto fileInfo : fileInfoList) {
        if (fileInfo.isFile()) {
            QString currentFileName = fileInfo.fileName();
            qInfo() << "Load the profile " << currentFileName;
            QFile *file = new QFile(CONFIGPATH + currentFileName);
            if (!file->open(QIODevice::ReadOnly)) {
                // 打开文件失败
                qCritical() << "fail to open file " << currentFileName;
                delete file;
                continue;
            }

            QByteArray data = file->readAll();
            QString strInfoAll = data;
            if (strInfoAll.isEmpty()) {
                qInfo() << currentFileName << " isEmpty";
                continue;
            }
            QByteArray infoJsonBuf = strInfoAll.toUtf8();
            QJsonDocument infoJsonDoc = QJsonDocument::fromJson(infoJsonBuf);
            QJsonObject infoJsonObj = infoJsonDoc.object();
            QString allKey = infoJsonObj.value(KEYWORD).toString();
            QString value = infoJsonObj.value(VALUEWORD).toString();
            QStringList keyList = allKey.split(BREAKSEP);
            for (auto key : keyList) {
                s_nomalMap.insert(key, value);
                s_tabletMap.insert(key, value);
            }
            file->close();
            delete file;
        }
    }
    if (s_nomalMap.isEmpty() || s_tabletMap.isEmpty()) {
        qWarning() << "There are no awakening keywords currently!";
    }
}

void StartUpTools::setModel(PublicAttributes::Model model)
{
    m_model = model;
}

QString StartUpTools::findKey(const QString &key)
{
    if (m_model == PublicAttributes::Model::Table) {
        if (!s_tabletMap.contains(key)) {
            return QString("");
        }
        QString path = s_tabletMap.find(key).value();

        QFileInfo file;
        QString filePath = path;
        if (path.contains(" ")) {
            filePath = path.mid(0, path.indexOf(" "));
        }
        if (!file.exists(filePath)) {
            return QString("");
        }

        return path;
    }

    if (!s_nomalMap.contains(key)) {
        return QString("");
    }
    QString path = s_nomalMap.find(key).value();

    QFileInfo file;
    QString filePath = path;
    if (path.contains(" ")) {
        filePath = path.mid(0, path.indexOf(" "));
    }
    if (!file.exists(filePath)) {
        return QString("");
    }

    return path;
}

void StartUpTools::startUp(const QString &path)
{
    qInfo() << "Start pulling up the app";
    QString command = path + " &";
    system(command.toStdString().c_str());
    qInfo() << "Add Data Burial Point";
    QString appName = path;
    if (path.contains(" ")) {
        appName = path.mid(0, path.indexOf(" "));
    }
    if (path.contains("/")) {
        appName = appName.mid(appName.lastIndexOf('/') + 1, appName.size());
    }

    Tools::addDiaNode(appName);
}