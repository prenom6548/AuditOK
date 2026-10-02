/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#include "bridge.h"
Bridge* Bridge::m_instance = nullptr;
QMutex Bridge::m_mutex;
Bridge::Bridge(QObject *parent) : QObject(parent)
{
}

Bridge *Bridge::getInstance(QObject *parent)
{
    if (m_instance == nullptr) {
        QMutexLocker locker(&m_mutex);
        if (m_instance == nullptr) {
            m_instance = new Bridge(parent);
        }
    }
    return m_instance;
}

void Bridge::wallpaperPathChanged(QString path)
{
    m_wallpaperpath = path;
//    m_configfilemanager->modifyWallPaperConf(path,QDir::homePath() + "/.cache/theme-build/"
//                                             +m_time + "/src/globalTheme/theme.conf");
    m_configfilemanager->copyPictoCacheDir(m_wallpaperpath,QDir::homePath()+"/.cache/theme-build/"
                                             +m_time+"/src/globalTheme","wallpaper");
    m_wallpaperpath = ":/resource/background/1-openkylin.jpg";
}


void Bridge::coverPathChanged(QString path)
{
    m_coverpath = path;
    m_configfilemanager->copyPictoCacheDir(m_coverpath,QDir::homePath()+"/.cache/theme-build/"
                                             +m_time+"/src/globalTheme","cover");
    m_coverpath = ":/resource/background/custom-preview.png";
}

void Bridge::radiusChanged(int radius)
{
    m_configfilemanager->modifyRadiusJson(radius,QDir::homePath() + "/.cache/theme-build/"
                                          +m_time + "/src/globalTheme/json.txt");
}

void Bridge::accentColorChanged(QColor accentcolor)
{
    m_configfilemanager->modifyAccentColorConf(accentcolor,QDir::homePath() + "/.cache/theme-build/"
                                               +m_time + "/src/globalTheme/theme.conf");
    QString ColorName;
    if(accentcolor.name() == "#3790fa"){
        ColorName = "daybreakBlue";
    }else if(accentcolor.name() == "#7873f5"){
        ColorName = "jamPurple";
    }else if(accentcolor.name() == "#eb3096"){
        ColorName = "magenta";
    }else if(accentcolor.name() == "#f3222d"){
        ColorName = "sunRed";
    }else if(accentcolor.name() == "#f68c27"){
        ColorName = "sunsetOrange";
    }else if(accentcolor.name() == "#f9c53d"){
        ColorName = "dustGold";
    }else if(accentcolor.name() == "#52c429"){
        ColorName = "polarGreen";
    }
    m_configfilemanager->modifyQtAccentColorConf(ColorName,QDir::homePath() + "/.cache/theme-build/"
                                                 +m_time + "/src/globalTheme/theme.conf");
}

void Bridge::exteriorChanged(QColor accentcolor)
{
    QString color = accentcolor.name(QColor::HexRgb);
    QString widgetStyle;
    if(color=="#1d1d1d"){
        widgetStyle = "dark";
    }else if(color=="#ffffff"){
        widgetStyle = "light";
    }
    m_configfilemanager->modifyWidgetStyleConf(widgetStyle,QDir::homePath() + "/.cache/theme-build/"
                                           +m_time + "/src/globalTheme/theme.conf");
}

void Bridge::transparencyChanged(int transparency)
{
    m_configfilemanager->modifyTransparencyConf(transparency,QDir::homePath() + "/.cache/theme-build/"
                                                +m_time + "/src/globalTheme/theme.conf");
}

void Bridge::windowRadiusChanged(int windowradius)
{
    m_configfilemanager->modifywindowRadiusConf(windowradius,QDir::homePath() + "/.cache/theme-build/"
                                                +m_time + "/src/globalTheme/theme.conf");
}

void Bridge::appIconsMapChanged(QMap<QString, QString> *appiconsmaps)
{
    m_appsiconpathmap = appiconsmaps;
    m_configfilemanager->copyIcontoCacheDir(m_appsiconpathmap,QDir::homePath()+"/.cache/theme-build/"
                                            +m_time+"/src/iconTheme/appicon");
}

void Bridge::systemIconsMapChanged(QMap<QString, QString> *systemiconsmaps)
{
    m_systemiconpathmap = systemiconsmaps;
    m_configfilemanager->copyIcontoCacheDir(m_systemiconpathmap,QDir::homePath()+"/.cache/theme-build/"
                                            +m_time+"/src/iconTheme/systemicon");
}

void Bridge::placeIconsMapChanged(QMap<QString, QString> *placeiconsmaps)
{
    m_placeiconpathmap = placeiconsmaps;
    m_configfilemanager->copyIcontoCacheDir(m_placeiconpathmap,QDir::homePath()+"/.cache/theme-build/"
                                            +m_time+"/src/iconTheme/placeicon");
}

void Bridge::deviceIconsMapChanged(QMap<QString, QString> *deviceiconpathmap)
{
    m_deviceiconpathmap = deviceiconpathmap;
    m_configfilemanager->copyIcontoCacheDir(m_deviceiconpathmap,QDir::homePath()+"/.cache/theme-build/"
                                            +m_time+"/src/iconTheme/deviceicon");
}

void Bridge::cursorMapChanged(QMap<QString, QString> *cursormap)
{
    m_cursorpathmap = cursormap;
    m_configfilemanager->copyIcontoCacheDir(cursormap,QDir::homePath()+"/.cache/theme-build/"
                                            +m_time+"/src/cursorTheme/cursor");
}

void Bridge::timeCursorMapChanged(QMap<QString, QString> *timecursormap)
{
    m_timecursorpathmap = timecursormap;
    m_configfilemanager->copyIcontoCacheDir(timecursormap,QDir::homePath()+"/.cache/theme-build/"
                                            +m_time+"/src/cursorTheme/timecursor");
}

void Bridge::plymouthPathChanged(QString path)
{
    m_plymouthpath = path;
    QFileInfo fileInfo(m_plymouthpath);
    if (fileInfo.suffix().toLower() == "mp4"){
        m_configfilemanager->copyMp4toCacheDir(m_plymouthpath,QDir::homePath()+"/.cache/theme-build/"
                                               +m_time+"/src/plymouthTheme");
    }else{
        m_configfilemanager->copyPicstoCacheDir(m_plymouthpath,QDir::homePath()+"/.cache/theme-build/"
                                               +m_time+"/src/plymouthTheme");
    }
}

void Bridge::grubPathChanged(QString path)
{
    m_grubpath = path;
    m_configfilemanager->copyPictoCacheDir(m_grubpath,QDir::homePath()+"/.cache/theme-build/"
                                           +m_time+"/src/grubTheme","grub");
}

void Bridge::savePathConfigChanged(QString path)
{
    m_configfilemanager->modifySavePathConf(path);
}

void Bridge::createConfig()
{
    m_configfilemanager->createConf(QDir::homePath() + "/.cache/theme-build/"
                                    + m_time + "/src/globalTheme/");
    m_configfilemanager->createJson(QDir::homePath() + "/.cache/theme-build/"
                                    + m_time + "/src/globalTheme/");
    m_configfilemanager->createXml(QDir::homePath() + "/.cache/theme-build/"
                                   + m_time + "/src/globalTheme/wallpaper/");
    m_configfilemanager->createFocalXml(QDir::homePath() + "/.cache/theme-build/"
                                        + m_time + "/src/globalTheme/wallpaper/");
    m_configfilemanager->copyPictoCacheDir(m_coverpath,QDir::homePath()+"/.cache/theme-build/"
                                           +m_time+"/src/globalTheme","cover");
    m_configfilemanager->copyPictoCacheDir(m_wallpaperpath,QDir::homePath()+"/.cache/theme-build/"
                                             +m_time+"/src/globalTheme","wallpaper");
}

void Bridge::createGrub()
{
    m_configfilemanager->copyPictoCacheDir(m_grubpath,QDir::homePath()+"/.cache/theme-build/"
                                           +m_time+"/src/grubTheme","grub");
}

void Bridge::createPlymouth()
{
    m_configfilemanager->copyAllPictoCacheDir(m_plymouthdir,QDir::homePath()+"/.cache/theme-build/"
                                              +m_time+"/src/plymouthTheme");
}

void Bridge::createSavePathConfig()
{
    m_configfilemanager->createSavePathConfig();
}

void Bridge::creatDesktop()
{
    m_configfilemanager->createDesktop(QDir::homePath()+"/.cache/theme-build/"
                                       +m_time+"/src/application/");
}

void Bridge::createSetupConf()
{
    m_configfilemanager->createSetupConf(QDir::homePath() + "/.cache/theme-build/"
                                                      + m_time + "/src/setupConfig/");
}

void Bridge::updateIconCache(QMap<QString, QString> *iconsmaps, QString icontype)
{
    if(icontype == "appicon"){
        m_configfilemanager->copyIcontoCacheDir(iconsmaps,QDir::homePath()+"/.cache/theme-build/"
                                                +m_time+"/src/iconTheme/appicon");
    }else if(icontype == "systemicon"){
        m_configfilemanager->copyIcontoCacheDir(iconsmaps,QDir::homePath()+"/.cache/theme-build/"
                                                +m_time+"/src/iconTheme/systemicon");
    }else if(icontype == "placeicon"){
        m_configfilemanager->copyIcontoCacheDir(iconsmaps,QDir::homePath()+"/.cache/theme-build/"
                                                +m_time+"/src/iconTheme/placeicon");
    }else if(icontype == "deviceicon"){
        m_configfilemanager->copyIcontoCacheDir(iconsmaps,QDir::homePath()+"/.cache/theme-build/"
                                                +m_time+"/src/iconTheme/deviceicon");
    }else if(icontype == "cursor"){
        m_configfilemanager->copyIcontoCacheDir(iconsmaps,QDir::homePath()+"/.cache/theme-build/"
                                                +m_time+"/src/cursorTheme/cursor");
    }else if(icontype == "timecursor"){
        m_configfilemanager->copyIcontoCacheDir(iconsmaps,QDir::homePath()+"/.cache/theme-build/"
                                                +m_time+"/src/cursorTheme/timecursor");
    }
}


void Bridge::ThemeNameChanged()
{
    m_configfilemanager->modifyFocalXml(QDir::homePath() + "/.cache/theme-build/"
                                        + m_time + "/src/globalTheme/wallpaper/kylin-theme-builder-focal.xml");
    m_configfilemanager->modifyDesktop(QDir::homePath() + "/.cache/theme-build/"
                                       + m_time + "/src/application/"+FileProcess::g_themeENName+".desktop");
}


void Bridge::createFileManager(QString time)
{
    m_time = time;
    m_configfilemanager = new ConfigFileManager();
//    createDir();
    connect(m_configfilemanager,&ConfigFileManager::updateInfo,this,&Bridge::updateInfo);
    connect(m_configfilemanager, &ConfigFileManager::updateThumbnail, this, [&](const QString& path) {
        emit updateThumbnail(path);
    });
}

//void Bridge::createDir()
//{
//    QString m_themePath = QDir::homePath() + "/.cache/theme-build/" +m_time;
//    QString m_themePathSrc = m_themePath + "/src";
//    QDir m_themePathSrcDir = m_themePathSrc;
////    m_builderConfig = m_themePathSrc + "/config";
//    m_builderappicon = m_themePathSrc + "/iconTheme" + "/appicon";
//    m_buildersystemicon = m_themePathSrc + "/iconTheme" + "/systemicon";
//    m_buildercursor = m_themePathSrc + "/cursorTheme" + "/cursor";
//    m_buildertimecursor = m_themePathSrc + "/cursorTheme" + "/timecursor";

//    m_themePathSrcDir.mkdir(m_themePathSrcDir.absolutePath());
//    m_builderConfig.mkdir(m_builderConfig.absolutePath());
//    m_buildericons.mkdir(m_buildericons.absolutePath());
//    m_builderappicon.mkdir(m_builderappicon.absolutePath());
//    m_buildersystemicon.mkdir(m_buildersystemicon.absolutePath());
//    m_buildercursors.mkdir(m_buildercursors.absolutePath());
//    m_buildercursor.mkdir(m_buildercursor.absolutePath());
//    m_buildertimecursor.mkdir(m_buildertimecursor.absolutePath());
//}
