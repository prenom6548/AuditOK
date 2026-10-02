#ifndef BRIDGE_H
#define BRIDGE_H
/*
 * Copyright (C) 2019 National University of Defense Technology(NUDT) & Kylin Ltd.
 *
 * Authors:
 *  xiaoyi wu    wuxiaoyi@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QKeyEvent>
#include <QTextStream>
#include <iostream>
#include <QDesktopWidget>
#include <QPainter>
#include <QComboBox>
#include <QScrollBar>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QApplication>
#include <QToolTip>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QCompleter>
#include <QPainterPath>
//#include <QSvgRenderer>

//#include "guide-widget.h"
//#include "main_controller.h"
#include "common-tool/comm_func.h"
//#include "common-tool/imageutil.h"

#include <QGSettings/QGSettings>
#define PANELMODEL_SETTINGS                 "org.ukui.SettingsDaemon.plugins.tablet-mode"
#define PANEL_MODEL                         "tablet-mode"

#include <QWebEngineView>
#include <QObject>

class Bridge : public QObject
{
Q_OBJECT
//首页app
Q_PROPERTY(QString getMdPathOther MEMBER m_someattribute NOTIFY mdPathOtherChanged)
//二级页面内容
Q_PROPERTY(QString getMdContent MEMBER m_someattribute NOTIFY mdContentChanged)

public:
    static Bridge* instance();
    Bridge();

public slots:
    QStringList js_getIntoFilename();
    QString system_name();
    QStringList getDirAndPng();
    QString js_getIndexMdFileInfo(QString appName);
    QString js_getIndexMdFilePathOther(QString appName);
    QString js_getIndexMdFileContent(QString IndexMdFilePath);
    QString js_getIndexMdFileTitle(QString IndexMdFilePath);
    QString js_getPageContent(QString appName);
    QString js_getFileSystemInformation(QString filePath);
    QString converRGB2HexStr(QColor _rgb);
    void js_executeCommand(QString cmd);
    void slot_SettingsChange(const QString &key);
    void js_changeNavHighlight();
    void js_openUrl(QUrl url);
    void getMdPathOther(QString attr);
    void getMdContent(QString attr);
    void getThemeColor(QString color);
    void js_buriedPoint(QString appName);

signals:
    void mdPathOtherChanged(QString attr);   //信号传参
    void mdContentChanged(QString attr);   //信号传参
    void showBackbutton();   //信号传参
    void highColorChange(QString color);
    

private:
    QString m_someattribute;
    QGSettings *settings;
    QLabel *m_pIconLabel = nullptr;
    QLabel *m_pTitleLabel = nullptr;
};

#endif // BRIDGE_H
