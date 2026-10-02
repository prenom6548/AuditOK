/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef BLACKLIST_H
#define BLACKLIST_H

#include <QStringList>

static const QStringList blackAppList() {
    QStringList l;
    //l<<"ukui-control-center";
    l<<"kylin-software-center.py";
    l<<"ubuntu-kylin-software-center.py";
//    l<<"kylin-burner";
    l<<"assistant";
    l<<"sogouIme-configtool";
    l<<"Ime Setting";
//    l<<"kylin-user-guide";
    l<<"biometric-authentication";
//    l<<"qtcreator";

    return l;
}

static const QStringList blackAppListWithBlurHelper() {
    QStringList l;
    l<<"youker-assistant";
//    l<<"kylin-assistant";
//    l<<"kylin-video";
//    l<<"ukui-control-center";
    l<<"ubuntu-kylin-software-center.py";
//    l<<"kylin-burner";
    l<<"ukui-clipboard";
    return l;
}

static const QStringList useDarkPaletteList() {
    //use dark palette in default style.
    QStringList l;
//    l<<"ukui-menu";
//    l<<"ukui-panel";
//    l<<"ukui-sidebar";
//    l<<"ukui-volume-control-applet-qt";
//    l<<"kylin-nm";                              //网络
//    l<<"panelukui-panel";                       //日历
//    l<<"ukui-power-manager-tray";
//    l<<"ukui-bluetooth";
//    l<<"sogouimebs";                            //输入法
//    l<<"kylin-device-daemon";                   //U盘
//    l<<"ukui-flash-disk";
//    l<<"ukui-bluetooth";
//    l<<"mktip";
//    l<<"kylin-video";

    return l;
}

static const QStringList useLightPaletteList() {
    //use light palette in default style.
    QStringList l;
//    l<<"kybackup";
//    l<<"biometric-manager";
//    l<<"kylin-video";
   l<<"ukui-screensaver-dialog";

    return l;
}

static const QStringList useTransparentButtonList() {
    //use transparent button
    QStringList l;
//    l<<"kybackup";
//    l<<"biometric-manager";
    l<<"kylin-video";
    l<<"kylin-ipmsg";
    l<<"kylin-weather";
    l<<"ukui-notebook";

//    l<<"kylin-recorder";

    return l;
}

static const QStringList focusStateActiveList() {
    //focus state style
    QStringList l;
    l<<"ukui-menu";
    l<<"platformthemeDemo";
    l<<"ukui-greeter-dialog";
    l<<"ukui-screensaver";

    return l;
}

static const QStringList windowManageBlackList() {
    QStringList l;
    l << "iflyime-spe-sym";
    l << "iflyime-qimpanel";
    l << "iflyime-setw";
    l << "iflyime-sett";
    l << "iflyime-qim";
    l << "iflyime-hw";
    l << "SpecificSymbol";
    return l;
}

// static const QStringList disableProgressBarAnimationList() {//不需要使用进度条动画的appName
//     QStringList l;
//     l << "ukui-settings-daemon";
//     return l;
// }

static const QStringList ukuiFiledialogBlackList() {
    QStringList l;
    l << "Notepad--";
    l << "mtxx";
    l << "keepassxc";

    return l;
}

static const QStringList ukuiXDPFiledialogList() {
    QStringList l;
    l << "qimgv";

    return l;
}


#endif // BLACKLIST_H
